/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
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

#include <OgreGLRenderSystem.h>
#include <OgreGLGpuProgram.h>
#include <OgreGLSLGpuProgram.h>
#include <OgreGLSLLinkProgramManager.h>
#include <OgreGLSLProgram.h>

#include <centralised_log.h>

#include "gfx_gl.h"

Ogre::RenderSystem *gfx_gl_get_render_system (void)
{
/*
    return nullptr;
*/
    return OGRE_NEW Ogre::GLRenderSystem();
}


void gfx_gl_force_shader_compilation(const Ogre::HighLevelGpuProgramPtr &vp,
                                     const Ogre::HighLevelGpuProgramPtr &fp)
{
	Ogre::GpuProgram *vp_bd = vp->_getBindingDelegate();
	Ogre::GpuProgram *fp_bd = fp->_getBindingDelegate();
	APP_ASSERT(vp_bd != nullptr);
	APP_ASSERT(fp_bd != nullptr);

    auto *vp_low = dynamic_cast<Ogre::GLSL::GLSLGpuProgram*>(vp_bd);
    auto *fp_low = dynamic_cast<Ogre::GLSL::GLSLGpuProgram*>(fp_bd);

    if (vp_low != nullptr && fp_low != nullptr) {
        // Force the actual compilation of it...
        Ogre::GLSL::GLSLLinkProgramManager::getSingleton().setActiveVertexShader(vp_low);
        Ogre::GLSL::GLSLLinkProgramManager::getSingleton().setActiveFragmentShader(fp_low);
        Ogre::GLSL::GLSLLinkProgramManager::getSingleton().getActiveLinkProgram();
    }
}
