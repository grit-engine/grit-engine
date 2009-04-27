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

        virtual void windowFocusChange(Ogre::RenderWindow *rw);

        virtual void windowClosed(Ogre::RenderWindow *rw);

        //virtual void doMesh(char *name);

        //virtual void doIPL(int ipl_count, char **ipl_names);

        virtual void doMain(int argc, const char** argv);

        virtual void render();

        virtual bool hasClickedClose();

        virtual Ogre::Root *getOgre();
        virtual Ogre::RenderWindow *getWin();
        virtual HUD::RootPtr getHUD();

        virtual Mouse *getMouse();

        virtual Keyboard *getKeyboard();

        virtual std::string toString();

        virtual UserDataTables& getUserDataTables();

        virtual GritObjectManager &getGOM() { return *gom; }

protected:

        Ogre::Root *ogre;

        HUD::RootPtr *hud;

        Mouse *mouse;

        Keyboard *keyboard;

        //Ogre::RaySceneQuery *rsq;

        bool clickedClose;

        bool hasTerminated;

        lua_State *L;

        UserDataTables userDataTables;

        GritObjectManager *gom;

};

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
