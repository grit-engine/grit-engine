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

class Grit;

extern Grit *grit;

#ifndef Grit_h
#define Grit_h

#include <Ogre.h>
#include <OgreSceneQuery.h>
#include <OgreOctreeSceneManager.h>

extern "C" {
#include "lua.h"
}

class Mouse;
class Keyboard;
namespace HUD {
        class Root;
        typedef Ogre::SharedPtr<Root> RootPtr;
}
class Streamer;

#include "BulletDebugDrawer.h"

#include "lua_userdata_dependency_tracker.h"

class Grit : public Ogre::WindowEventListener, public Ogre::MeshSerializerListener {

public:

        Grit (Ogre::Root *root, Mouse *mouse, Keyboard *keyboard, Grit *& grit);

        virtual ~Grit ();

        virtual void windowResized(Ogre::RenderWindow *rw);

        virtual bool windowHasFocus (void);

        virtual void windowClosed (Ogre::RenderWindow *rw);

        virtual void doMain (int argc, const char** argv);

        virtual void render (void);

        virtual bool hasClickedClose (void) { return clickedClose; }

        virtual Ogre::Root *getOgre (void) { return ogre; }
        virtual Ogre::OctreeSceneManager *getSM (void) { return sm; }
        virtual BulletDebugDrawer *getDebugDrawer (void) { return debugDrawer; }
        virtual Ogre::RenderWindow *getWin (void);
        virtual HUD::RootPtr getHUD (void);

        virtual Mouse *getMouse (void) { return mouse; }

        virtual Keyboard *getKeyboard (void) { return keyboard; }

        virtual std::string toString (void);

        virtual UserDataTables& getUserDataTables (void) { return userDataTables; }

        virtual Streamer &getStreamer() { return *streamer; }

        virtual lua_State *getLuaState (void) const { return L; }

        virtual void processMaterialName (Ogre::Mesh *mesh, Ogre::String *name);
        virtual void processSkeletonName (Ogre::Mesh *mesh, Ogre::String *name);

protected:

        Ogre::Root *ogre;

        Ogre::OctreeSceneManager *sm;

        HUD::RootPtr *hud;

        Mouse *mouse;

        Keyboard *keyboard;

        bool clickedClose;

        bool hasTerminated;

        lua_State *L;

        UserDataTables userDataTables;

        Streamer *streamer;

        BulletDebugDrawer *debugDrawer;
};

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
