class Grit;

extern Grit *grit;

#ifndef Grit_h
#define Grit_h

#include <Ogre.h>
#include <OgreSceneQuery.h>

extern "C" {
#include "lua.h"
}

class Mouse;
class Keyboard;
namespace HUD {
        class Root;
        typedef Ogre::SharedPtr<Root> RootPtr;
}
class GritObjectManager;

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

        virtual bool hasClickedClose (void);

        virtual Ogre::Root *getOgre (void);
        virtual Ogre::RenderWindow *getWin (void);
        virtual HUD::RootPtr getHUD (void);

        virtual Mouse *getMouse (void);

        virtual Keyboard *getKeyboard (void);

        virtual std::string toString (void);

        virtual UserDataTables& getUserDataTables (void);

        virtual GritObjectManager &getGOM() { return *gom; }

        virtual lua_State *getLuaState (void) const { return L; }

protected:

        Ogre::Root *ogre;

        HUD::RootPtr *hud;

        Mouse *mouse;

        Keyboard *keyboard;

        bool clickedClose;

        bool hasTerminated;

        lua_State *L;

        UserDataTables userDataTables;

        GritObjectManager *gom;

};

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
