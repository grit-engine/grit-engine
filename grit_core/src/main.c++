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
#else
#  include "linux/KeyboardX11.h"
#  include "linux/MouseX11.h"
#endif

#include "Grit.h"
#include "BackgroundMeshLoader.h"
#include "app_error.h"

Grit *grit = NULL;

#include "console_colour.h"

void app_verbose(char const* file, int line, const std::string& msg)
{
        std::cout<<BOLD GREEN"VERBOSE "RESET
                 <<BOLD<<file<<NOBOLD":"BOLD<<line<<NOBOLD
                 <<": \""BOLD BLUE<<msg<<RESET"\"";
        std::cout<<std::endl;
}

void app_error(char const* file, int line,
               const std::string& i_was, const std::string& msg)
{
        std::cout<<BOLD RED"ERROR "RESET
                 <<BOLD<<file<<NOBOLD":"BOLD<<line<<NOBOLD
                 <<": \""BOLD YELLOW<<msg<<RESET"\"";
        if (i_was!="")
                std::cout<<" ("BOLD YELLOW<<i_was<<RESET")";
        std::cout<<std::endl;
}

void app_line(const std::string &msg)
{
        std::cout<<BOLD<<msg<<NOBOLD<<std::endl;
}

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


                #ifdef NO_PLUGINS
                        Ogre::Root* ogre = OGRE_NEW Ogre::Root("");

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
                        Ogre::Root* ogre = OGRE_NEW Ogre::Root();
                #endif

                BackgroundMeshLoader *bml = new BackgroundMeshLoader();



                if (!ogre->restoreConfig()) {
                        APP_ERROR("Failure","reading ogre.cfg");
                        if (!ogre->showConfigDialog()) {
                                APP_ERROR("Failure","showing config dialogue");
                                app_fatal();
                        }
                }


                Ogre::RenderWindow *win = ogre->initialise(true,"Grit");


                size_t winid;
                win->getCustomAttribute("WINDOW", &winid);

                #ifdef WIN32
                Mouse *mouse = new MouseDirectInput8(winid);
                Keyboard *keyb = new KeyboardDirectInput8(winid);
                #else
                Mouse *mouse = new MouseX11(winid);
                Keyboard *keyb = new KeyboardX11(winid);
                #endif


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
