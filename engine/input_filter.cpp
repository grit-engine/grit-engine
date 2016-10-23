/* Copyright (c) The Grit Game Engine authors 2016
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "lua_util.h"
#include "lua_ptr.h"
#include "input_filter.h"
#include "gfx/gfx_hud.h"

typedef InputFilter::BindingSet BindingSet;
typedef std::map<double, InputFilter *> IFMap;
static IFMap ifmap;

static std::set<std::string> pressed;
bool input_filter_pressed (const std::string &button)
{
    return pressed.find(button) != pressed.end();
}


static Vector2 last_mouse_pos(0,0);
static Vector2 prior_grab_mouse_pos(0,0);
static bool is_captured = false;
static bool cursor_hidden = false;

// Based on the state of the input filters, grab or ungrab the mouse.
static void update_grabbed (void)
{
    bool should_be_captured = false;
    for (IFMap::iterator i=ifmap.begin(),i_=ifmap.end() ; i!=i_ ; ++i) {
        InputFilter *f = i->second;
        if (!f->getEnabled()) continue;
        if (f->getMouseCapture()) should_be_captured = true;
        if (f->getModal()) break;
    }
    if (!keyboard->hasFocus()) should_be_captured = false;
    bool should_hide = should_be_captured || cursor_hidden;
    if (is_captured == should_be_captured) {
        mouse->setHide(should_hide);
        return;
    }
    is_captured = should_be_captured;
    if (should_be_captured) {
        prior_grab_mouse_pos = last_mouse_pos;
        mouse->setHide(should_hide);
        mouse->setGrab(true);
    } else {
        mouse->setPos(prior_grab_mouse_pos.x, prior_grab_mouse_pos.y);
        last_mouse_pos = prior_grab_mouse_pos;
        mouse->setGrab(false);
        mouse->setHide(should_hide);
    }
}

bool input_filter_has (double order)
{
    IFMap::iterator i = ifmap.find(order);
    return i != ifmap.end();
}

std::string input_filter_get_description (double order)
{
    IFMap::iterator i = ifmap.find(order);
    if (i == ifmap.end()) EXCEPT << "No InputFilter at order " << order << ENDL;
    return i->second->description;
}

namespace {
    struct Combo {
        bool ctrl, alt, shift;
        std::string prefix;
        Combo (void) { }
        Combo (bool ctrl, bool alt, bool shift, const std::string &prefix)
          : ctrl(ctrl), alt(alt), shift(shift), prefix(prefix) { }
    };

    Combo combos[8] = {
        Combo(true, true, true, "C+A+S+"),
        Combo(true, true, false, "C+A+"),
        Combo(true, false, true, "C+S+"),
        Combo(false, true, true, "A+S+"),
        Combo(true, false, false, "C+"),
        Combo(false, true, false, "A+"),
        Combo(false, false, true, "S+"),
        Combo(false, false, false, ""),
    };
    const Combo &empty_combo = combos[7];
}

static const Combo &prefix_to_combo (const std::string &prefix)
{
    for (unsigned i=0 ; i<8 ; ++i) {
        const Combo &c = combos[i];
        if (c.prefix == prefix) return c;
    }
    EXCEPT << "Unrecognised key combo: " << prefix << ENDL;
}

bool InputFilter::isBound (const std::string &ev)
{
    return buttonCallbacks.find(ev) != buttonCallbacks.end();
}

std::vector<std::string> InputFilter::allBinds (void)
{
    std::vector<std::string> r;
    for (const auto &pair : buttonCallbacks) {
        r.push_back(pair.first);
    }
    return r;
}

bool InputFilter::acceptButton (lua_State *L, const std::string &ev)
{
    ensureAlive();
    char kind = ev[0];
    std::string button = &ev[1];

    if (kind == '+') {

        std::vector<Combo> found;
        for (unsigned i=0 ; i<8 ; ++i) {
            const Combo &c = combos[i];
            // ignore prefixes that do not apply given current modifier status
            if (c.ctrl && !input_filter_pressed("Ctrl")) continue;
            if (c.alt && !input_filter_pressed("Alt")) continue;
            if (c.shift && !input_filter_pressed("Shift")) continue;
            if (isBound(c.prefix+button)) found.push_back(c);
        }
        // if no bindings match, fall through
        if (found.size() == 0) return false;

        // should not already be down
        APP_ASSERT(!isButtonPressed(button));

        const Combo &c = found[0];
        
        down[button] = c.prefix;

        std::string bind = c.prefix + button;

        const Callback &cb = buttonCallbacks.find(bind)->second;
        
        const LuaPtr &func = cb.down;

        if (!func.isNil()) {
            triggerFunc(L, bind, func);
        }

    } else if (kind == '-' || kind == '=') {

        ButtonStatus::iterator earlier = down.find(button);

        // Maybe some lower filter has it bound with fewer modifiers.
        if (earlier == down.end()) return false;

        const std::string &prefix = earlier->second;

        std::string bind = prefix + button;

        // If it's down, there must be a callback for it
        const Callback &cb = buttonCallbacks.find(bind)->second;

        if (kind == '-') {
            down.erase(button);
        }

        const LuaPtr &func = kind == '-' ? cb.up : cb.repeat;
        if (!func.isNil()) {
            triggerFunc(L, bind, func);
        }

    }

    return true;
}

void InputFilter::triggerFunc (lua_State *L, const std::string &bind, const LuaPtr &func)
{
    ensureAlive();

    APP_ASSERT(!func.isNil());

    STACK_BASE;
    //stack is empty

    // error handler in case there is a problem during 
    // the callback
    push_cfunction(L, my_lua_error_handler);
    int error_handler = lua_gettop(L);

    //stack: err

    // get the function
    func.push(L);
    //stack: err,callback

    STACK_CHECK_N(2);

    // call (1 arg), pops function too
    int status = lua_pcall(L,0,0,error_handler);
    if (status) {
        STACK_CHECK_N(2);
        //stack: err,error
        // pop the error message since the error handler will
        // have already printed it out
        lua_pop(L,2);
        STACK_CHECK;
        CERR << "InputFilter \"" << description << "\" "
             << "raised an error on bind "<<bind<<"." << std::endl;
        //stack is empty
    } else {
        //stack: err
        STACK_CHECK_N(1);
        lua_pop(L,1);
        //stack is empty
    }

    //stack is empty
    STACK_CHECK;
}

void InputFilter::triggerMouseMove (lua_State *L, const Vector2 &rel)
{
    ensureAlive();

    STACK_BASE;
    //stack is empty

    // error handler in case there is a problem during 
    // the callback
    push_cfunction(L, my_lua_error_handler);
    int error_handler = lua_gettop(L);

    //stack: err

    // get the function
    mouseMoveCallback.push(L);
    //stack: err,callback
    if (lua_isnil(L,-1)) {
        lua_pop(L,2);
        STACK_CHECK;
        CERR << "InputFilter \"" << description << "\" "
             << "has no mouseMoveCallback function, releasing mouse." << std::endl;
        setMouseCapture(L, false);
        return;
    }


    //stack: err,callback
    STACK_CHECK_N(2);

    push_v2(L, rel);
    //stack: err,callback,rel

    STACK_CHECK_N(3);

    // call (1 arg), pops function too
    int status = lua_pcall(L,1,0,error_handler);
    if (status) {
        STACK_CHECK_N(2);
        //stack: err,error
        // pop the error message since the error handler will
        // have already printed it out
        lua_pop(L,2);
        STACK_CHECK;
        CERR << "InputFilter \"" << description << "\" "
             << "raised an error on mouseMoveCallback, disabling mouse capture." << std::endl;
        setMouseCapture(L, false);
        //stack is empty
    } else {
        //stack: err
        STACK_CHECK_N(1);
        lua_pop(L,1);
        //stack is empty
    }

    //stack is empty
    STACK_CHECK;
}

void InputFilter::flushAll (lua_State *L)
{
    ensureAlive();
    // 'down' modified by acceptButton (and also potentially by callbacks).
    ButtonStatus d = down;
    for (ButtonStatus::iterator i=d.begin(),i_=d.end() ; i!=i_ ; ++i) {
        acceptButton(L, "-"+i->first);
    }
    APP_ASSERT(down.size() == 0);
}

/** Is maskee masked by masker? */
static bool masked_by (const Combo &maskee, const Combo &masker)
{
    // masker needs to have fewer modifiers
    if (masker.ctrl && !maskee.ctrl) return false;
    if (masker.alt && !maskee.alt) return false;
    if (masker.shift && !maskee.shift) return false;
    return true;
}

static std::pair<Combo,std::string> split_bind (const std::string &bind)
{
    // This code assumes keys do not contain - and are non-empty.
    size_t last = bind.rfind('-');
    if (last == std::string::npos) return std::pair<Combo,std::string>(empty_combo, bind);
    APP_ASSERT(last+1 < bind.length());
    std::string prefix = bind.substr(0, last+1);
    std::string button = bind.substr(last+1);
    return std::pair<Combo,std::string>(prefix_to_combo(prefix), button);
}

static bool masked_by (const std::string &maskee_, const std::string &masker_)
{
    auto maskee = split_bind(maskee_);
    auto masker = split_bind(masker_);

    // Different keys, cannot mask.
    if (maskee.second != masker.second) return false;

    // Otherwise examine key combinations.
    return masked_by(maskee.first, masker.first);
}

/** Is the given bind maskee masked by something in the set masker? */
static bool masked_by (const std::string &maskee, const BindingSet &masker)
{
    for (BindingSet::iterator i=masker.begin(),i_=masker.end() ; i!=i_ ; ++i) {
        if (masked_by(maskee, *i)) return true;
    }
    return false;
}

void InputFilter::flushSet (lua_State *L, const BindingSet &s)
{
    ensureAlive();
    // 'down' modified by acceptButton (and also potentially by callbacks).
    ButtonStatus d = down;
    for (ButtonStatus::iterator i=d.begin(),i_=d.end() ; i!=i_ ; ++i) {
        if (masked_by(i->second, s))
            acceptButton(L, "-"+i->first);
    }
    APP_ASSERT(down.size() == 0);
}

void input_filter_trickle_button (lua_State *L, const std::string &ev)
{
    char kind = ev[0];
    std::string button = &ev[1];
    if (kind == '+') {
        pressed.insert(button);
    } else if (kind == '-') {
        pressed.erase(button);
    }

    // copy because callbacks can alter the map while we're iterating over it
    IFMap m = ifmap;
    bool captured = false;
    for (IFMap::const_iterator i=m.begin(),i_=m.end() ; i!=i_ ; ++i) {
        InputFilter *f = i->second;
        if (!f->getEnabled()) continue;
        if (f->getMouseCapture()) captured = true;
        if (f->acceptButton(L, ev)) break;
        if (f->getModal()) break;
    }
    if (!captured) gfx_hud_signal_button(L, ev);
}

void input_filter_trickle_mouse_move (lua_State *L, const Vector2 &rel, const Vector2 &abs)
{
    // copy because callbacks can alter the map while we're iterating over it
    last_mouse_pos = abs;
    IFMap m = ifmap;
    for (IFMap::const_iterator i=m.begin(),i_=m.end() ; i!=i_ ; ++i) {
        InputFilter *f = i->second;
        if (!f->getEnabled()) continue;
        if (f->getMouseCapture()) {
            f->triggerMouseMove(L, rel);
            return;
        }
        if (f->getModal()) break;
    }
    gfx_hud_signal_mouse_move(L, abs);
}

void input_filter_flush (lua_State *L)
{
    pressed.clear();
    IFMap m = ifmap;
    for (IFMap::const_iterator i=m.begin(),i_=m.end() ; i!=i_ ; ++i) {
        InputFilter *f = i->second;
        f->flushAll(L);
    }
    gfx_hud_signal_flush(L);
    update_grabbed();
}

std::vector<std::pair<double,std::string>> input_filter_list (void)
{
    std::vector<std::pair<double,std::string>> r;
    for (IFMap::const_iterator i=ifmap.begin(),i_=ifmap.end() ; i!=i_ ; ++i) {
        r.emplace_back(i->second->order, i->second->description);
    }
    return r;
}

void input_filter_set_cursor_hidden (bool v)
{
    cursor_hidden = v;
    update_grabbed();
}

bool input_filter_get_cursor_hidden (void)
{
    return cursor_hidden;
}

void input_filter_shutdown (lua_State *L)
{
    IFMap m = ifmap;
    for (IFMap::const_iterator i=m.begin(),i_=m.end() ; i!=i_ ; ++i) {
        InputFilter *f = i->second;
        f->destroy(L);
    }
    APP_ASSERT(ifmap.size() == 0);
}


InputFilter::InputFilter (double order, const std::string &desc)

  : modal(false), enabled(true), mouseCapture(false), destroyed(false), order(order),
    description(desc)
{
    IFMap::iterator i = ifmap.find(order);
    if (i != ifmap.end()) {
        EXCEPT << "Cannot create InputFilter \"" << desc << "\" at order already occupied by "
               << "\""<<i->second->description<<"\"" << std::endl;
    }
    ifmap[order] = this;
}
    
InputFilter::~InputFilter (void)
{
    if (!destroyed) {
        CERR << "InputFilter \"" << description << "\" deleted without being destroyed first." << std::endl;
        mouseMoveCallback.leak();
        CallbackMap &m = buttonCallbacks;
        for (CallbackMap::iterator i=m.begin(),i_=m.end() ; i!=i_ ; ++i) {
            i->second.down.leak();
            i->second.up.leak();
            i->second.repeat.leak();
        }
        buttonCallbacks.clear();
        ifmap.erase(this->order);
    }
}

void InputFilter::destroy (lua_State *L)
{
    ensureAlive();
    destroyed = true;
    mouseMoveCallback.setNil(L);
    CallbackMap &m = buttonCallbacks;
    for (CallbackMap::iterator i=m.begin(),i_=m.end() ; i!=i_ ; ++i) {
        i->second.down.setNil(L);
        i->second.up.setNil(L);
        i->second.repeat.setNil(L);
    }
    buttonCallbacks.clear();
    ifmap.erase(this->order);
}

void InputFilter::bind (lua_State *L, const std::string &button)
{
    ensureAlive();

    Callback &c = buttonCallbacks[button];
    c.down.setNil(L);
    c.up.setNil(L);
    c.repeat.setNil(L);
    APP_ASSERT(lua_gettop(L) >= 3);
    // DOWN
    if (!lua_isnil(L, -3)) {
        if (!lua_isfunction(L, -3)) {
            EXCEPT << "Expected a function for down binding of " << button << ENDL;
        }
        c.down.setNoPop(L, -3);
    }
    // UP
    if (!lua_isnil(L, -2)) {
        if (!lua_isfunction(L, -2)) {
            EXCEPT << "Expected a function for up binding of " << button << ENDL;
        }
        c.up.setNoPop(L, -2);
    }
    // REPEAT
    if (!lua_isnil(L, -1)) {
        if (lua_isboolean(L, -1)) {
            if (lua_toboolean(L, -1)) {
                if (lua_isnil(L, -3)) {
                    c.repeat.setNoPop(L, -3);
                }
            }
            // it's false, treat the same as nil
        } else {
            if (!lua_isfunction(L, -1)) {
                EXCEPT << "Expected a function or true for repeat binding of " << button << ENDL;
            }
            c.repeat.setNoPop(L, -1);
        }
    }
}

void InputFilter::unbind (lua_State *L, const std::string &button)
{
    ensureAlive();

    Callback &c = buttonCallbacks[button];
    c.down.setNil(L);
    c.up.setNil(L);
    c.repeat.setNil(L);
    buttonCallbacks.erase(button);
    // just in case the button was down at the time of the unbind call
    down.erase(button);
}

void InputFilter::setMouseMoveCallback (lua_State *L)
{
    ensureAlive();
    mouseMoveCallback.setNoPop(L);
}

void InputFilter::setEnabled (lua_State *L, bool v)
{
    ensureAlive();
    if (enabled == v) return;
    enabled = v;
    if (enabled) {
        // If enabling, release all masked buttons below.
        BindingSet masked;
        for (auto i=buttonCallbacks.begin(),i_=buttonCallbacks.end() ; i!=i_ ; ++i) {
            masked.insert(i->first);
        }
        IFMap::iterator i = ifmap.find(this->order);
        i++;
        for ( ; i!=ifmap.end(); ++i) {
            i->second->flushSet(L, masked);
        }
    } else {
        // If disabling, release all local buttons.
        flushAll(L);
    }

    enabled = v;
    update_grabbed();
}

void InputFilter::setModal (lua_State *L, bool v)
{
    ensureAlive();
    if (modal == v) return;
    modal = v;
    if (modal) {
        IFMap::iterator i = ifmap.find(this->order);
        i++;
        for ( ; i!=ifmap.end(); ++i) {
            i->second->flushAll(L);
        }
    }
    update_grabbed();
}

void InputFilter::setMouseCapture (lua_State *L, bool v)
{
    ensureAlive();
    if (mouseCapture == v) return;
    mouseCapture = v;

    if (mouseCapture) {
        gfx_hud_signal_flush(L);
    }
    update_grabbed();
}

bool InputFilter::isButtonPressed (const std::string &b)
{
    ensureAlive();
    return down.find(b) != down.end();
}
