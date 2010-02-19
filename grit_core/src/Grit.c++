/* Copyright (c) David Cunningham and the Grit Game Engine project 2010
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

#include <assert.h>
#include <errno.h>
#include <float.h>

#include <vector>

#include <Ogre.h>
#include <OgreFontManager.h>
#include <OgreMeshManager.h>
#include <OgreOverlayElementFactory.h>
#include <OgreAxisAlignedBox.h>

#include "Grit.h"
#include "sleep.h"
#include "ray.h"
#include "CentralisedLog.h"
#include "lua_util.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "lua_wrappers_core.h"
#include "GritObject.h"
#include "Streamer.h"
#include "path_util.h"

#include "HUD.h"

class TextListOverlayElementFactory : public Ogre::OverlayElementFactory {

        public:

        virtual Ogre::OverlayElement*
        createOverlayElement (const Ogre::String& instanceName)
        {
            return new HUD::TextListOverlayElement(instanceName);
        }

        virtual const Ogre::String& getTypeName () const
        {
            return HUD::TextListOverlayElement::msTypeName;
        }

};

Grit::Grit (Ogre::Root *ogre, Mouse *mouse, Keyboard *keyboard, Grit *& grit) :
        ogre(ogre),
        hud(NULL),
        mouse(mouse),
        keyboard(keyboard),
        clickedClose(false),
        hasTerminated(false),
        L(NULL),
        streamer(new Streamer())
{

        Ogre::OverlayManager::getSingleton()
                .addOverlayElementFactory(new TextListOverlayElementFactory());

        hud = new HUD::RootPtr(new HUD::Root(getWin()->getWidth(),
                                             getWin()->getHeight()));

        grit = this;

        sm = static_cast<Ogre::OctreeSceneManager*>(ogre->createSceneManager("OctreeSceneManager"));

        debugDrawer = new BulletDebugDrawer(sm);

        //APP_VERBOSE("adding event listener");
        Ogre::WindowEventUtilities::addWindowEventListener(getWin(), this);
        //APP_VERBOSE("done adding event listener");

        Ogre::MeshManager::getSingleton().setListener(this);

        Ogre::ResourceGroupManager::getSingleton().
                addResourceLocation(".","FileSystem","GRIT",true);

        L = init_lua("/system/init.lua");

}

Grit::~Grit ()
{
        hasTerminated = true;
        streamer->doShutdown(L);
        delete streamer;
        if (hud) delete hud;
        if (L) lua_close(L);
        if (mouse) delete mouse;
        if (keyboard) delete keyboard;
        if (debugDrawer) delete debugDrawer;
        if (sm && ogre) ogre->destroySceneManager(sm);
        if (ogre) OGRE_DELETE ogre;
}

void Grit::processMaterialName (Ogre::Mesh *mesh, Ogre::String *name)
{
        std::string filename = mesh->getName();
        std::string dir(filename, 0, filename.rfind('/')+1);
        *name = pwd_full_ex(*name, "/"+dir, "BaseWhite");
}

void Grit::processSkeletonName (Ogre::Mesh *mesh, Ogre::String *name)
{
        std::string filename = mesh->getName();
        std::string dir(filename, 0, filename.rfind('/')+1);
        *name = pwd_full_ex(*name, "/"+dir, *name).substr(1); // strip leading '/' from this one
}

Ogre::RenderWindow *Grit::getWin () {
        return ogre->getAutoCreatedWindow();
}

HUD::RootPtr Grit::getHUD () {
        return HUD::RootPtr(*hud);
}

std::string Grit::toString () {
        std::stringstream r;
        r << "Grit: " << (void*)this;
        return r.str();
}

void Grit::windowResized (Ogre::RenderWindow* rw)
{
        (void) rw;
        if (hasTerminated) return;
        (*hud)->parentResized(getWin()->getWidth(),getWin()->getHeight());
}

void Grit::windowClosed (Ogre::RenderWindow *rw)
{
        (void) rw;
        clickedClose = true;
}

bool Grit::windowHasFocus (void)
{
        return keyboard->hasFocus();
}

void Grit::render ()
{
        ogre->renderOneFrame();
}

void Grit::doMain (int argc, const char **argv)
{
        int error_handler;

        push_cfunction(L, my_lua_error_handler);
        error_handler = lua_gettop(L);

        lua_getglobal(L, "main");
        if (lua_isnil(L,-1)) {
                lua_pop(L,1); // nil 'main object'
                lua_pop(L,1); //handler
                CERR << "invoking main:run(...): "
                     << "object \"main\" not found";
                app_fatal();
        }

        lua_getfield(L, -1, "run");
        if (lua_isnil(L,-1)) {
                lua_pop(L,1); // nil 'run function'
                lua_pop(L,1); //handler
                CERR << "invoking main:run(...): "
                     << "function \"run\" not found";
                app_fatal();
        }

        // arguments
        lua_getglobal(L, "main");
        for (int i=0 ; i<argc ; i++) {
                check_stack(L,1);
                lua_pushstring(L,argv[i]);
        }

        int status = lua_pcall(L, argc+1, 0, error_handler);
        if (status) {
                lua_pop(L,1); // error message
        }

        lua_pop(L,1); // error_handler
}

// vim: shiftwidth=8:tabstop=8:expandtab
