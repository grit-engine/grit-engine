//#include <fenv.h>
#include <errno.h>

#include <Ogre.h>
#include <OgreArchiveFactory.h>

#ifdef NO_PLUGINS
#  include "OgreOctreePlugin.h"
#  include "OgreGLPlugin.h"
#  include "OgreParticleFXPlugin.h"
#  ifdef WIN32
#    include "OgreD3D9Plugin.h"
#  endif
#endif

#ifdef WIN32
#  include "win32/MouseDirectInput8.h"
#  include "win32/KeyboardDirectInput8.h"
#  include "win32/KeyboardWinAPI.h"
#else
#  include "linux/KeyboardX11.h"
#  include "linux/MouseX11.h"
#endif

#include "Grit.h"
#include "BackgroundMeshLoader.h"
#include "CentralisedLog.h"
#include "LuaParticleSystem.h"

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
                bool use_dinput = getenv("GRIT_DINPUT")!=NULL;
                #else
                bool use_d3d9 = false;
                #endif

                bool use_vsync = getenv("GRIT_NOVSYNC")==NULL;
                bool use_fullscreen = getenv("GRIT_FULLSCREEN")!=NULL;

                if (use_d3d9) {
                        Ogre::RenderSystem *rs =
                                ogre->getRenderSystemByName("Direct3D9 Rendering Subsystem");
                        rs->setConfigOption("Allow NVPerfHUD","Yes");
                        //rs->setConfigOption("Anti aliasing","None");
                        rs->setConfigOption("Floating-point mode","Consistent");
                        rs->setConfigOption("Full Screen",use_fullscreen?"Yes":"No");
                        rs->setConfigOption("VSync",use_vsync?"Yes":"No");
                        rs->setConfigOption("Video Mode","800 x 600 @ 32-bit colour");
                        rs->setConfigOption("sRGB Gamma Conversion","No");
                        ogre->setRenderSystem(rs);
                } else {
                        Ogre::RenderSystem *rs =
                                ogre->getRenderSystemByName("OpenGL Rendering Subsystem");
                        rs->setConfigOption("FSAA","0");
                        rs->setConfigOption("Full Screen",use_fullscreen?"Yes":"No");
                        rs->setConfigOption("RTT Preferred Mode","FBO");
                        rs->setConfigOption("VSync",use_vsync?"Yes":"No");
                        rs->setConfigOption("Video Mode","800 x 600");
                        rs->setConfigOption("sRGB Gamma Conversion","No");
                        ogre->setRenderSystem(rs);
                }

                Ogre::RenderWindow *win = ogre->initialise(true,"Grit Game Window");

                size_t winid;
                win->getCustomAttribute("WINDOW", &winid);

                #ifdef WIN32
                Mouse *mouse = new MouseDirectInput8(winid);
                Keyboard *keyb = use_dinput ? (Keyboard *)new KeyboardDirectInput8(winid)
                                            : (Keyboard *)new KeyboardWinAPI(winid);
                HMODULE mod = GetModuleHandle(NULL);
                HICON icon = LoadIcon(mod,MAKEINTRESOURCE(111));
                SendMessage((HWND)winid, (UINT)WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) icon);
                SendMessage((HWND)winid, (UINT)WM_SETICON, (WPARAM) ICON_SMALL, (LPARAM) icon);
                #else
                Mouse *mouse = new MouseX11(winid);
                Keyboard *keyb = new KeyboardX11(winid);
                #endif

                Ogre::ParticleSystemManager::getSingleton()
                        .addAffectorFactory(new LuaParticleAffectorFactory());

                Ogre::ParticleSystemManager::getSingleton()
                        .addRendererFactory(new LuaParticleRendererFactory());

                new Grit(ogre,mouse,keyb,grit); // writes itself back to grit

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
                        #ifdef WIN32
                                OGRE_DELETE d3d9;
                        #endif
                #endif


        } catch( Ogre::Exception& e ) {
                std::cerr << "An exception has occured: "
                          << e.getFullDescription() << std::endl;
        }
        return EXIT_SUCCESS;
}

// vim: shiftwidth=8:tabstop=8:expandtab
