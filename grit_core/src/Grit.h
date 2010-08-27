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

        Grit (Ogre::Root *root, Grit *& grit);

        ~Grit ();

        void onWindowUpdate (void);

        void windowResized(Ogre::RenderWindow *rw);

        bool windowHasFocus (void);

        void windowClosed (Ogre::RenderWindow *rw);

        void doMain (int argc, const char** argv);

        void render (void);

        bool hasClickedClose (void) { return clickedClose; }

        Ogre::Root *getOgre (void) { return ogre; }
        Ogre::OctreeSceneManager *getSM (void) { return sm; }
        BulletDebugDrawer *getDebugDrawer (void) { return debugDrawer; }
        Ogre::RenderWindow *getWin (void);
        HUD::RootPtr getHUD (void);

        Mouse *getMouse (void) { return mouse; }

        Keyboard *getKeyboard (void) { return keyboard; }

        std::string toString (void);

        UserDataTables& getUserDataTables (void) { return userDataTables; }

        Streamer &getStreamer() { return *streamer; }

        lua_State *getLuaState (void) const { return L; }

        void processMaterialName (Ogre::Mesh *mesh, std::string *name);
        void processSkeletonName (Ogre::Mesh *mesh, std::string *name);

protected:

        Ogre::Root *ogre;

        Ogre::RenderWindow *win;

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

        int initialisationFailed;
};

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
