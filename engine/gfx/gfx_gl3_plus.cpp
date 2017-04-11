/* Copyright (c) The Grit Game Engine authors 2016
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

#include <OgreGL3PlusRenderSystem.h>
#include <OgreGLSLSeparableProgramManager.h>

#include <centralised_log.h>

#include "gfx_gl3_plus.h"

Ogre::RenderSystem *gfx_gl3_plus_get_render_system (void)
{
    return OGRE_NEW Ogre::GL3PlusRenderSystem();
}

void gfx_gl3_plus_force_shader_compilation(const Ogre::HighLevelGpuProgramPtr &vp,
                                           const Ogre::HighLevelGpuProgramPtr &fp)
{
    auto *vp_low = dynamic_cast<Ogre::GLSLShader*>(&*vp);
    auto *fp_low = dynamic_cast<Ogre::GLSLShader*>(&*fp);

    if (vp_low != nullptr && fp_low != nullptr) {
        // Force the actual compilation of it...
        Ogre::GLSLSeparableProgramManager::getSingleton().setActiveVertexShader(vp_low);
        Ogre::GLSLSeparableProgramManager::getSingleton().setActiveFragmentShader(fp_low);
        auto *prog =
            Ogre::GLSLSeparableProgramManager::getSingleton().getCurrentSeparableProgram();
        if (intptr_t(prog->getComputeShader()) > 0 && intptr_t(prog->getComputeShader()) < 10)
            abort();
    }
}
