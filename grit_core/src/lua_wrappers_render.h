#include "lua_wrappers_common.h"

//class Ogre::RenderTexture;
//class Ogre::RenderWindow;
//class Ogre::Viewport;

#define RTEX_TAG "Grit/RenderTexture"
MT_MACRO_DECLARE(rtex);
void push_rtex (lua_State *L, Ogre::RenderTexture *self) ;

#define RWIN_TAG "Grit/RenderWindow"
MT_MACRO_DECLARE(rwin);
void push_rwin (lua_State *L, Ogre::RenderWindow *self);

#define VIEWPORT_TAG "Grit/Viewport"
MT_MACRO_DECLARE(viewport);
void push_viewport (lua_State *L, Ogre::Viewport *vp);



// vim: shiftwidth=8:tabstop=8:expandtab
