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

#include <map>
#include <vector>
#include <set>
#include <string>

#include "shared_ptr.h"

class GritObject;
typedef SharedPtr<GritObject> GritObjectPtr;
typedef std::map<std::string, GritObjectPtr> GObjMap;
typedef std::vector<GritObjectPtr> GObjPtrs;
typedef std::set<GritObjectPtr> GObjSet;


#ifndef GritObject_h
#define GritObject_h

#include "background_loader.h"
#include "external_table.h"
#include "streamer.h"

/** A streamed game object, occupying a place in the map.
 *
 * Grit bjects are instances of Grit Classes (\see GritClass).  The class is a
 * blueprint of the object, and is intended define the majority of the object's
 * qualities and behaviour.  Once created, they are either in a 'deactivated'
 * state, meaning the player is outside the object's rendering distance, or
 * 'activated' otherwise.  The intent is that when deactivated, the object is
 * in a low-resource state where very little memory and no CPU resources are
 * required.  In particular, no Lua heap resources are used when the object is
 * deactivated, as all its state is stored in an ExternalTable.  This means
 * garbage collection remains fast as the map scales up.  There are a number of
 * Lua callbacks that are issued by the object when its state changes or when
 * other interesting events occur.  These callbacks are the basis of user
 * scripts to create active objects, i.e. objects that appear alive or react to
 * stimuli.
 *
 * When the object is activated, it has a Lua table which holds its state and
 * is maintained by the Lua scripts that define its behaviour.  When it is
 * deactivated, this table is destroyed and the only persistent state is held
 * in the External table in the object itself.
 *
 * The object is typically referred to via a reference counting smart pointer,
 * and many of the member functions in this class require that smart pointer to
 * be passed in, becuase they need to pass it on to Lua or whatever.  In such
 * cases the parameter is called 'self' and it is essentially the same as
 * 'this', except for having the refernece counter information attached.
 *
 * TODO: talk about lods
 */ 
class GritObject {

    public:

    /** Don't use this, create an object with object_add. */
    GritObject (const std::string &name_, GritClass *gritClass_);

    /** Destructor. */
    ~GritObject (void) { }

    /** Explicit destroy call, invokes Lua callbacks via Lua state.  Once
     * destroyed, the object no-longer plays a role in the game, although it still
     * exists in memory.  Its state is marked as being destroyed by setting the
     * class to NULL.  Attempts to use a destroyed class result in exceptions. */
    void destroy (lua_State *L, const GritObjectPtr &self);

    /** Calls Lua deactivate callbacks. */
    bool deactivate (lua_State *L, const GritObjectPtr &self);

    /** Calls Lua init callbacks. */
    void init (lua_State *L, const GritObjectPtr &self);

    /** Calls Lua activate callbacks. */
    void activate (lua_State *L, const GritObjectPtr &self);

    /** Calculates the extent (0.0f to 1.0f) that this object is visible,
     * based on the range, its rendering distance, and its lod associations.  Also
     * returns whether or not the object is 'overlapping' i.e. making the
     * transition with another lod. */
    float calcFade (const float range2, bool &overlap);

    /** Called to tell the object that its distance to the player has changed. */
    void notifyRange2 (lua_State *L, const GritObjectPtr &self, const float range2)
    {
        if (gritClass==NULL) GRIT_EXCEPT("Object destroyed");
        bool overlap = false;
        float fade = calcFade(range2, overlap);
        // if fade is sufficiently different
        if (fade!=lastFade) {
            notifyFade(L, self, fade);
            lastFade = fade;
        }
    }

    /** Call the Lua per-frame callback, with the given elapsed time
     * quantity.  A frame is a discrete unit of graphical display. */
    bool frameCallback (lua_State *L,
                        const GritObjectPtr &self,
                        const float time);

    /** Call the Lua per-game-tick callback, with the given elapsed time
     * quantity.  A step is a unit of game logic, including rigid body motion
     * calculations.*/
    bool stepCallback (lua_State *L,
                       const GritObjectPtr &self,
                       const float time);

    /** Call the Lua fade callback. 
     *
     * This is called whenever the object changes its fade state (0.0f to 1.0f).
     */
    void notifyFade (lua_State *L,
                     const GritObjectPtr &self,
                     const float fade);

    /** Is the object activated (close enough to the player)? */
    bool isActivated (void) const { return lua != LUA_NOREF; }

    /** Push the object's Lua table state, which only exists when it is activated. */
    void pushLuaTable (lua_State *L)
    {
         lua_rawgeti(L, LUA_REGISTRYINDEX, lua);
    }


    /** Add a resource that must be loaded before this object is activated. */
    void addDiskResource (const std::string &name)
    { demand.addDiskResource(name); }

    /** Clear this list of dependent disk resources that must be loaded
     * before this object is activated. */
    void clearDiskResources (void)
    { demand.clearDiskResources(); }

    /** Request that the resource's disk resources be loaded.  The cam_pos
     * is required in order to prioritise the loading when many requests are made
     * concurrently. */
    bool requestLoad (const Vector3 &cam_pos)
    {
        return demand.requestLoad((cam_pos - pos).length2());
    }

    /** Indicate that a previous request to load should now be cancelled,
     * and that the resources are no-longer needed.  They may be unloaded if system
     * resource pressure requires it.
     */
    void tryUnloadResources(void)
    {
        demand.finishedWith();
    }

    /** Are the resources associated with this object queued to be loaded.
     * The loading happens in a background thread which is held up by disk I/O and
     * a queue of resources that need loading.
     */
    bool isInBackgroundQueue (void)
    {
        return demand.isInBackgroundQueue();
    }

    /** Return whether or not there was a problem loading dependent resources. */
    bool backgroundLoadingCausedError (void)
    {
        return demand.errorOnLoad();
    }

    /** Reload all the disk resources that this object depends on.  This is
     * a convenience function for development, when reloading e.g. the textures can
     * be useful for an instant preview. */
    void reloadDiskResources(void)
    {
        demand.immediateReload();
    }


    /** The class that this object is an instance of. */
    GritClass *getClass (void) { return gritClass; }

    /** How faded out the object is (0.0f meaning fully faded out, 1.0f
     * fully faded in.  Fading is for rendering distance and lod transitions. */
    float getFade (void) const { return lastFade; }

    /** Whether or not this object should have its frame callback invoked every frame. */
    bool getNeedsFrameCallbacks (void) const { return needsFrameCallbacks; }
    /** Whether or not this object should have its frame callback invoked every frame. */
    void setNeedsFrameCallbacks (const GritObjectPtr &self, bool v);

    /** Whether or not this object should have its step callback invoked every frame. */
    bool getNeedsStepCallbacks (void) const { return needsStepCallbacks; }
    /** Whether or not this object should have its step callback invoked every frame. */
    void setNeedsStepCallbacks (const GritObjectPtr &self, bool v);

    /** The object's name. */
    const std::string name;

    /** The RangeSpace datastructure requires that object remember its
     * index within that structure.  Presence of this function is required of any
     * c++ class that is to be used in a RangeSpace. */
    inline void updateIndex (int index_)
    {
        index = index_;
    }

    /** Update the position and rendering distance of this object. */
    void updateSphere (const Vector3 &pos, float r_);
    /** Update just the position of this object. */
    void updateSphere (const Vector3 &pos);
    /** Update just the rendering distance of this object. */
    void updateSphere (float r_);

    /** Return the current position of this object in the game map.  If
     * deactivated, this is the spawn location of the object.  If activated, it is
     * wherever it has moved to since being activated. */
    Vector3 getPos() const { return pos; }

    /** Return the rendering distance of this object. */
    float getR() const { return r; }

            
    /** Get the 'near' version of this object, for lod transitions. */
    const GritObjectPtr &getNearObj (void) const { return nearObj; }

    /** Set the 'near' version of this object, for lod transitions. */
    void setNearObj (const GritObjectPtr &self, const GritObjectPtr &v)
    {
        // this function is recursive and mutually recursive
        // but always terminates due to the early return below
        if (nearObj==v) return;
        if (nearObj.isNull()) {
            // note that v cannot be null since near!=v
            // adding a near
            nearObj = v;
            nearObj->setFarObj(nearObj, self);
        } else {
            if (v.isNull()) {
                // removing near
                GritObjectPtr tmp = nearObj;
                nearObj.setNull();
                tmp->setFarObj(tmp, nearObj);
            } else {
                // changing nearObj
                // first set to null
                GritObjectPtr tmp = nearObj;
                nearObj.setNull();
                tmp->setFarObj(tmp, nearObj);
                // then set to new value
                setNearObj(self, v);
            }
        }
    }


    /** Get the 'far' version of this object, used for lod transitions. */
    const GritObjectPtr &getFarObj (void) const { return farObj; }

    /** Set the 'far' version of this object, used for lod transitions. */
    void setFarObj (const GritObjectPtr &self, const GritObjectPtr &v)
    {
        // this function is recursive and mutually recursive
        // but always terminates due to the early return below
        if (farObj==v) return;
        if (farObj.isNull()) {
            // adding a far
            farObj = v;
            farObj->setNearObj(farObj, self);
        } else {
            if (v.isNull()) {
                // removing far
                GritObjectPtr tmp = farObj;
                farObj.setNull();
                tmp->setNearObj(tmp, farObj);
            } else {
                // changing far
                // first set to null
                GritObjectPtr tmp = farObj;
                farObj.setNull();
                tmp->setNearObj(tmp, farObj);
                // then set to new value
                setFarObj(self, v);
            }
        }
    }

    /** Return the squared distance to the given point. */
    float range2 (const Vector3 &cam_pos) const
    {
        return (cam_pos - pos).length2() / (r*r);
    }

    /** Is the obejct within range of the given point, using the given
     * global scaling factor on rendering distances. */
    bool withinRange (const Vector3 &cam_pos, float factor) const
    {
        const float rad = r*factor;
        return (cam_pos - pos).length2() < rad*rad;
    }

    /** If you are the 'far' lod for this object, you must fade out by this
     * amount because you are transitioning with the 'near' lod. */
    float getImposedFarFade (void) const { return imposedFarFade; }

    /** The state of the object that persists even after it deactivates. */
    ExternalTable userValues;

    /** Whether or not this object had a generated name. */
    bool anonymous;

    void getField (lua_State *L, const std::string &f) const;

    protected:

    /** Current position of the object. */
    Vector3 pos;

    /** Current rendering distance. */
    float r;
            
    /** The object's class. */
    GritClass *gritClass;

    /** A pointer to the object's state in Lua (when activated).  This
     * could be replaced with a LuaPtr, it currently directly uses the Lua
     * registry. */
    int lua;

    /** The index of the object within the range space. */
    int index;

    /** Whether or not the callbacks should be called. */
    bool needsFrameCallbacks;

    /** Whether or not the callbacks should be called. */
    bool needsStepCallbacks;

    /** The lod companion that should be activated when the player is closer to the object. */
    GritObjectPtr nearObj;

    /** The lod companion that should be activated when the player is outside of this object's
     * rendering distance.
     */
    GritObjectPtr farObj;
            
    /** Appears to be unused? */
    bool demandRegistered;
    Demand demand;

    /** The fade imposed upon a far lod companion because we are the near lod and we are taking
     * over.
     */
    float imposedFarFade;

    /** Used to tell if the fade has changed when recalculating it.  The
     * callback is only invoked if the fade has changed, for performance reasons.
     */
    float lastFade;
};

/** Instantiate the given class to create a new object with the given name.
 * The lua state is used for callbacks and errors, if any. */
GritObjectPtr object_add (lua_State *L, std::string name, GritClass *grit_class);

/** Destroy the object, which removes it from the game. */
void object_del (lua_State *L, const GritObjectPtr &o);

/** Get a non-destroyed object by name. */
const GritObjectPtr &object_get (const std::string &name);

/** Does an object exist with this name? */
bool object_has (const std::string &name);
    
/** Return all objects, via the two iterator 'byref' params. */
void object_all (GObjMap::iterator &begin, GObjMap::iterator &end);

/** Clear all objects from the game map. */
void object_all_del (lua_State *L);

/** Return the number of objects that currently exist. */
int object_count (void);

/** Call the frame callbacks for all objects, providing the given elapsed time quantity. */
void object_do_frame_callbacks (lua_State *L, float elapsed);

/** Call the step callbacks for all objects, providing the given elapsed time quantity. */
void object_do_step_callbacks (lua_State *L, float elapsed);

#endif

// vim: shiftwidth=4:tabstop=4:expandtab
