#include <assert.h>
#include <errno.h>
#include <float.h>

#include <vector>

#include <Ogre.h>
#include <OgreFontManager.h>
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
#include "GritObjectManager.h"

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
        gom(new GritObjectManager())
{

        Ogre::OverlayManager::getSingleton()
                .addOverlayElementFactory(new TextListOverlayElementFactory());

        hud = new HUD::RootPtr(new HUD::Root(getWin()->getWidth(),
                                             getWin()->getHeight()));

        grit = this;

        L = init_lua("lua_base/main.lua");

        //APP_VERBOSE("adding event listener");
        Ogre::WindowEventUtilities::addWindowEventListener(getWin(), this);
        //APP_VERBOSE("done adding event listener");
}

Grit::~Grit ()
{
        hasTerminated = true;
        gom->doShutdown(L);
        delete gom;
        if (hud) delete hud;
        if (L) lua_close(L);
        if (mouse) delete mouse;
        if (keyboard) delete keyboard;
        if (ogre) OGRE_DELETE ogre;
}

Ogre::Root *Grit::getOgre () {
        return ogre;
}

Ogre::RenderWindow *Grit::getWin () {
        return ogre->getAutoCreatedWindow();
}

HUD::RootPtr Grit::getHUD () {
        return HUD::RootPtr(*hud);
}

Keyboard *Grit::getKeyboard () {
        return keyboard;
}

Mouse *Grit::getMouse () {
        return mouse;
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

void Grit::render ()
{
        ogre->renderOneFrame();
}

bool Grit::hasClickedClose ()
{
        return clickedClose;
}


/*
        Ogre::Light *light = scnmgr->createLight("Sky");
        light->setType(Ogre::Light::LT_DIRECTIONAL);
        light->setDirection(Ogre::Vector3(0, 0, -1));

        light->setDiffuseColour( 0.0, 0, 0 );
        light->setSpecularColour( 0, 0.0, 0 );
*/

/*
        rsq = scnmgr->createRayQuery(Ogre::Ray());
        rsq->setSortByDistance(true);
*/

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


UserDataTables& Grit::getUserDataTables ()
{
        return userDataTables;
}

// vim: shiftwidth=8:tabstop=8:expandtab
