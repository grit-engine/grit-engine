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

class Grit : public Ogre::WindowEventListener {

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
