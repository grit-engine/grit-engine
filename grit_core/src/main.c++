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

//#include <fenv.h>
#include <cerrno>

#include <Ogre.h>
#include <OgreArchiveFactory.h>
#include <OgreOverlayElementFactory.h>

#ifdef NO_PLUGINS
#  include "OgreOctreePlugin.h"
#  include "OgreGLPlugin.h"
#  include "OgreParticleFXPlugin.h"
#  include "OgreCgPlugin.h"
#  ifdef WIN32
#    include "OgreD3D9Plugin.h"
#  endif
#endif

#include "Grit.h"
#include "BackgroundMeshLoader.h"
#include "CentralisedLog.h"
#include "LuaParticleSystem.h"
#include "Clutter.h"
#include "HUD.h"

CentralisedLog clog;

Grit *grit = NULL;

#include "console_colour.h"

int already_fatal = 0;

void app_fatal()
{
        if (!already_fatal) {
                already_fatal = 1;
                delete grit;
        }

        abort();
}

// TODO: move this somewhere sensible
class TextListOverlayElementFactory : public Ogre::OverlayElementFactory {

        public:

        virtual Ogre::OverlayElement*
        createOverlayElement (const std::string& instanceName)
        {
            return new HUD::TextListOverlayElement(instanceName);
        }

        virtual const std::string& getTypeName () const
        {
            return HUD::TextListOverlayElement::msTypeName;
        }

};

int main(int argc, const char **argv)
{

        try {


                Ogre::LogManager *lmgr = OGRE_NEW Ogre::LogManager();
                Ogre::Log *ogre_log = OGRE_NEW Ogre::Log("",false,true);
                ogre_log->addListener(&clog);
                lmgr->setDefaultLog(ogre_log);
                lmgr->setLogDetail(Ogre::LL_NORMAL);

                #ifdef FE_NOMASK_ENV
                //feenableexcept(FE_INVALID);
                //feenableexcept(FE_DIVBYZERO | FE_INVALID);
                #endif

                #ifdef NO_PLUGINS
                        Ogre::Root* ogre = OGRE_NEW Ogre::Root("","","");

                        Ogre::GLPlugin *gl = OGRE_NEW Ogre::GLPlugin();
                        ogre->installPlugin(gl);

                        Ogre::OctreePlugin *octree =
                                OGRE_NEW Ogre::OctreePlugin();
                        ogre->installPlugin(octree);

                        Ogre::ParticleFXPlugin *pfx =
                                OGRE_NEW Ogre::ParticleFXPlugin();
                        ogre->installPlugin(pfx);

                        Ogre::CgPlugin *cg =
                                OGRE_NEW Ogre::CgPlugin();
                        ogre->installPlugin(cg);

                        #ifdef WIN32
                        Ogre::D3D9Plugin *d3d9 = OGRE_NEW Ogre::D3D9Plugin();
                        ogre->installPlugin(d3d9);
                        #endif
                #else
                        Ogre::Root* ogre = OGRE_NEW Ogre::Root("plugins.cfg","","");
                #endif

                BackgroundMeshLoader *bml = new BackgroundMeshLoader();

                #ifdef WIN32
                bool use_d3d9 = getenv("GRIT_GL")==NULL;
                #else
                bool use_d3d9 = false;
                #endif

                bool use_hwgamma = getenv("GRIT_NOHWGAMMA")==NULL;

                Ogre::RenderSystem *rs;
                if (use_d3d9) {
                        rs = ogre->getRenderSystemByName("Direct3D9 Rendering Subsystem");
                        rs->setConfigOption("Allow NVPerfHUD","Yes");
                        rs->setConfigOption("Floating-point mode","Consistent");
                        rs->setConfigOption("Video Mode","800 x 600 @ 32-bit colour");
                } else {
                        rs = ogre->getRenderSystemByName("OpenGL Rendering Subsystem");
                        rs->setConfigOption("RTT Preferred Mode","FBO");
                        rs->setConfigOption("Video Mode","800 x 600");
                }
                rs->setConfigOption("sRGB Gamma Conversion",use_hwgamma?"Yes":"No");
                rs->setConfigOption("Full Screen","No");
                rs->setConfigOption("VSync","Yes");
                ogre->setRenderSystem(rs);

                ogre->initialise(true,"Grit Game Window");

                Ogre::ParticleSystemManager::getSingleton()
                        .addAffectorFactory(new LuaParticleAffectorFactory());
                Ogre::ParticleSystemManager::getSingleton()
                        .addRendererFactory(new LuaParticleRendererFactory());
                Ogre::OverlayManager::getSingleton()
                        .addOverlayElementFactory(new TextListOverlayElementFactory());
                ogre->addMovableObjectFactory(new MovableClutterFactory());
                ogre->addMovableObjectFactory(new RangedClutterFactory());


                new Grit(ogre,grit); // writes itself back to grit

                // now call out to lua

                grit->doMain(argc,argv);

                // lua returns - we quit

                bml->shutdown();

                delete grit;

                // ogre was deleted inside the grit destructor above

                delete bml;

                #ifdef NO_PLUGINS
                        OGRE_DELETE gl;
                        OGRE_DELETE octree;
                        OGRE_DELETE pfx;
                        OGRE_DELETE cg;
                        #ifdef WIN32
                                OGRE_DELETE d3d9;
                        #endif
                #endif


        } catch( GritException& e ) {
                std::cerr << "An exception has occured: "
                          << e.longMessage() << std::endl;
        } catch( Ogre::Exception& e ) {
                std::cerr << "An exception has occured: "
                          << e.getFullDescription() << std::endl;
        }
        return EXIT_SUCCESS;
}

// vim: shiftwidth=8:tabstop=8:expandtab
