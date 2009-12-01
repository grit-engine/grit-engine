#include <OgreGpuProgram.h>

#include "lua_wrappers_common.h"


#define GPUPROG_TAG "Grit/GPUProg"
MT_MACRO_DECLARE(gpuprog);

void push_gpuprog (lua_State *L, const Ogre::HighLevelGpuProgramPtr &self);


// vim: shiftwidth=8:tabstop=8:expandtab
