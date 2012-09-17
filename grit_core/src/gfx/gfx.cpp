/* Copyright (c) David Cunningham and the Grit Game Engine project 2012 *
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

#include <sstream>

#include <OgreAutoParamDataSource.h>

#include "../path_util.h"

#include "gfx_internal.h"
#include "GfxPipeline.h"
#include "gfx_option.h"
#include "Clutter.h"
#include "GfxMaterial.h"
#include "GfxBody.h"
#include "GfxSkyMaterial.h"
#include "GfxSkyBody.h"
#include "../sleep.h"

#ifdef WIN32
bool d3d9 = getenv("GRIT_GL")==NULL;
#else
bool d3d9 = false;
#endif

Ogre::OctreePlugin *octree;
Ogre::CgPlugin *cg;

Ogre::Root *ogre_root = NULL;
Ogre::RenderSystem *ogre_rs = NULL;
Ogre::RenderWindow *ogre_win = NULL;
Ogre::OctreeSceneManager *ogre_sm = NULL;
Ogre::SceneNode *ogre_root_node = NULL;
Ogre::SharedPtr<Ogre::FrameTimeControllerValue> ftcv;

Ogre::SceneNode *ogre_sky_node = NULL;
Ogre::Light *ogre_sun = NULL;

GfxCallback *gfx_cb = NULL;
bool shutting_down = false;
bool use_hwgamma = false; //getenv("GRIT_NOHWGAMMA")==NULL;

// For default parameters of functions that take GfxStringMap
const GfxStringMap gfx_empty_string_map;

Ogre::Matrix4 shadow_view_proj[3];

Vector3 fog_colour;
float fog_density;
Vector3 sky_light_colour(0,0,0);

std::string env_cube_name;
Ogre::TexturePtr env_cube_tex;
float env_brightness = 1;
float global_exposure = 1;
float global_contrast = 0;

// abuse ogre fog params to store several things
static void set_ogre_fog (void)
{
    ogre_sm->setFog(Ogre::FOG_EXP2, to_ogre_cv(fog_colour), fog_density, env_brightness, global_exposure);
}


GfxMaterialDB material_db;
fast_erase_vector<GfxBody*> gfx_all_bodies;


// {{{ utilities

int freshname_counter = 0;
std::string freshname (const std::string &prefix)
{
    std::stringstream ss;
    ss << prefix << freshname_counter;
    freshname_counter++;
    return ss.str();
}
std::string freshname (void)
{
    return freshname("F:");
}

// }}}



void do_reset_framebuffer (void)
{
    // get rid of everything that might be set up already

    delete eye_left;
    eye_left = NULL;
    delete eye_right;
    eye_right = NULL;

    // set things up again
    eye_left = new GfxPipeline("EyeLeft", ogre_win, true);

    if (stereoscopic()) {
        eye_right = new GfxPipeline("EyeRight", ogre_win, false);
    }

    do_reset_eyes();

}



// {{{ SCENE PROPERTIES

Vector3 gfx_sun_get_diffuse (void)
{
    return from_ogre_cv(ogre_sun->getDiffuseColour());;
}

void gfx_sun_set_diffuse (const Vector3 &v)
{
    ogre_sun->setDiffuseColour(to_ogre_cv(v));
}

Vector3 gfx_sun_get_specular (void)
{
    return from_ogre_cv(ogre_sun->getSpecularColour());;
}

void gfx_sun_set_specular (const Vector3 &v)
{
    ogre_sun->setSpecularColour(to_ogre_cv(v));
}

Vector3 gfx_sun_get_direction (void)
{
    return from_ogre(ogre_sun->getDirection());
}

void gfx_sun_set_direction (const Vector3 &v)
{
    ogre_sun->setDirection(to_ogre(v));
}

Vector3 gfx_particle_ambient_get (void)
{
    return from_ogre_cv(ogre_sm->getAmbientLight());
}

void gfx_particle_ambient_set (const Vector3 &v)
{
    ogre_sm->setAmbientLight(to_ogre_cv(v));
}

std::string gfx_env_cube_get (void)
{
    return env_cube_name;
}

void gfx_env_cube_set (const std::string &v)
{
    Ogre::Image disk;
    Ogre::Image img;
    uint8_t *raw_tex = NULL;
    if (v == "") {
        // Prepare texture (6(1x1) RGB8)
        uint8_t raw_tex_stack[] = {
            0xff, 0x7f, 0x7f,
            0x00, 0x7f, 0x7f,
            0x7f, 0xff, 0x7f,
            0x7f, 0x00, 0x7f,
            0x7f, 0x7f, 0xff,
            0x7f, 0x7f, 0x00
        };
        // Load raw byte array into an Image
        img.loadDynamicImage(raw_tex_stack, 1, 1, 1, Ogre::PF_B8G8R8, false, 6, 0);
    } else if (v.substr(v.length()-4) == ".dds") {
        // Reorganise into 
        img.load (v, RESGRP);
    } else {
        // Reorganise into 
        disk.load (v, RESGRP);
        if (disk.getWidth() != disk.getHeight()*6) {
            GRIT_EXCEPT("Environment map has incorrect dimensions: "+v);
        }
        unsigned sz = disk.getHeight();
        if (sz & (sz-1) ) {
            GRIT_EXCEPT("Environment map dimensions not a power of 2: "+v);
        }
        raw_tex = new uint8_t[disk.getSize()];
        img.loadDynamicImage(raw_tex, sz, sz, 1, disk.getFormat(), false, 6, 0);
        // copy faces across
        Ogre::PixelBox from = disk.getPixelBox();
        for (unsigned face=0 ; face<6 ; ++face) {
            Ogre::PixelBox to = img.getPixelBox(face,0);
            for (unsigned y=0 ; y<sz ; ++y) {
                for (unsigned x=0 ; x<sz ; ++x) {
                    to.setColourAt(from.getColourAt(face*sz + x, y, 0), x, y, 0);
                }
            }
        }
    }

    // Create texture based on img
    if (env_cube_tex.isNull()) {
        env_cube_tex = Ogre::TextureManager::getSingleton().loadImage(ENV_CUBE_TEXTURE, RESGRP, img, Ogre::TEX_TYPE_CUBE_MAP);
    } else {
        env_cube_tex->unload();
        env_cube_tex->setTextureType(Ogre::TEX_TYPE_CUBE_MAP);
        env_cube_tex->loadImage(img);
    }

    env_cube_name = v;

    delete [] raw_tex;
}

float gfx_env_brightness_get (void)
{
    return env_brightness;
}

void gfx_env_brightness_set (float v)
{
    env_brightness = v;
    set_ogre_fog();
}
    
float gfx_global_contrast_get (void)
{
    return global_contrast;
}

void gfx_global_contrast_set (float v)
{
    global_contrast = v;
    set_ogre_fog();
}
    
float gfx_global_exposure_get (void)
{
    return global_exposure;
}

void gfx_global_exposure_set (float v)
{
    global_exposure = v;
    set_ogre_fog();
}
    


Vector3 gfx_fog_get_colour (void)
{
    return fog_colour;
}

void gfx_fog_set_colour (const Vector3 &v)
{
    fog_colour = v;
    set_ogre_fog();
}

float gfx_fog_get_density (void)
{
    return fog_density;
}

void gfx_fog_set_density (float v)
{
    fog_density = v;
    set_ogre_fog();
}

// }}}


// {{{ RENDER

void update_coronas (const Vector3 &cam_pos);

static float time_since_started_rendering = 0;

void gfx_render (float elapsed, const Vector3 &cam_pos, const Quaternion &cam_dir)
{
    time_since_started_rendering += elapsed;

    try {
        // This pumps ogre's texture animation and probably other things
        ftcv->setValue(elapsed);
        ftcv->setElapsedTime(time_since_started_rendering);

        Ogre::WindowEventUtilities::messagePump();

        update_coronas(cam_pos);

        handle_dirty_materials();
        handle_dirty_sky_materials();

        (void) elapsed;
        // something with elapsed, for texture animation, etc

        if (ogre_win->isActive()) {
            eye_left->render(cam_pos, cam_dir);
            if (stereoscopic())
                eye_right->render(cam_pos, cam_dir);

            ogre_rs->_swapAllRenderTargetBuffers(true);
        } else {
            // corresponds to 100fps
            mysleep(10000);
        }


    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT2(e.getFullDescription(), "Rendering a frame");
    }
}

// }}}


void gfx_screenshot (const std::string &filename) { ogre_win->writeContentsToFile(filename); }

static GfxLastRenderStats stats_from_rt (Ogre::RenderTarget *rt)
{
    GfxLastRenderStats r;
    r.batches = float(rt->getBatchCount());
    r.triangles = float(rt->getTriangleCount());
    return r;
}

GfxLastFrameStats gfx_last_frame_stats (void)
{
    GfxLastFrameStats r;

    r.left_deferred = eye_left->getDeferredStats();
    r.left_gbuffer = eye_left->getGBufferStats();

    if (stereoscopic()) {
        r.right_deferred = eye_right->getGBufferStats();
        r.right_gbuffer = eye_right->getGBufferStats();
    }

    if (gfx_option(GFX_SHADOW_CAST)) {
        for (int i=0 ; i<3 ; ++i) {
            r.shadow[i] = stats_from_rt(ogre_sm->getShadowTexture(i)->getBuffer()->getRenderTarget());
        }
    }

    return r;
}

GfxRunningFrameStats gfx_running_frame_stats (void)
{
    GfxRunningFrameStats r;
    return r;
}


// {{{ LISTENERS 

struct MeshSerializerListener : Ogre::MeshSerializerListener {
    void processMaterialName (Ogre::Mesh *mesh, std::string *name)
    {
        if (shutting_down) return;
        std::string filename = mesh->getName();
        std::string dir(filename, 0, filename.rfind('/')+1);
        *name = pwd_full_ex(*name, "/"+dir, "BaseWhite");
    }

    void processSkeletonName (Ogre::Mesh *mesh, std::string *name)
    {
        if (shutting_down) return;
        std::string filename = mesh->getName();
        std::string dir(filename, 0, filename.rfind('/')+1);
        *name = pwd_full_ex(*name, "/"+dir, *name).substr(1); // strip leading '/' from this one
    }
} mesh_serializer_listener;

struct WindowEventListener : Ogre::WindowEventListener {

    void windowResized(Ogre::RenderWindow *rw)
    {
        if (shutting_down) return;
        gfx_cb->windowResized(rw->getWidth(),rw->getHeight());
        do_reset_framebuffer();
    }

    void windowClosed (Ogre::RenderWindow *rw)
    {
        (void) rw;
        if (shutting_down) return;
        gfx_cb->clickedClose();
    }

} window_event_listener;

struct LogListener : Ogre::LogListener {
    virtual void messageLogged (const std::string &message,
                                Ogre::LogMessageLevel lml,
                                bool maskDebug,
                                const std::string &logName,
                                bool& skipThisMessage )

    {
        (void)lml;
        (void)logName;
        (void)skipThisMessage;
        if (!maskDebug) gfx_cb->messageLogged(message);
    }
} log_listener;

struct SceneManagerListener : Ogre::SceneManager::Listener {
    //virtual void preUpdateSceneGraph (Ogre::SceneManager *, Ogre::Camera *camera)
    //{
    //    //CVERB << "preUpdateSceneGraph: " << camera->getName() << std::endl;
    //}

    //virtual void preFindVisibleObjects (Ogre::SceneManager *, Ogre::SceneManager::IlluminationRenderStage irs, Ogre::Viewport *v)
    //{
    //    //CVERB << "preFindVisibleObjects: " << irs << " " << v << std::endl;
    //}

    //virtual void postFindVisibleObjects (Ogre::SceneManager *, Ogre::SceneManager::IlluminationRenderStage irs, Ogre::Viewport *v)
    //{
    //    //CVERB << "postFindVisibleObjects: " << irs << " " << v << std::endl;
    //}

    virtual void shadowTexturesUpdated (size_t numberOfShadowTextures)
    {
        (void) numberOfShadowTextures;
        // misleading, actually refers to number of lights that have had shadow textures populated i think
        //APP_ASSERT(numberOfShadowTextures==1);
        //if (numberOfShadowTextures!=1)
        //    CVERB << "shadowTexturesUpdated: " << numberOfShadowTextures << std::endl;
    }

    virtual void shadowTextureCasterPreViewProj (Ogre::Light *light, Ogre::Camera *cam, size_t iteration)
    {
        // apparently other lights cast shadows, should probably fix that...
        if (light != ogre_sun) return;
        APP_ASSERT(iteration < 3);
        //CVERB << "shadowTextureCasterPreViewProj: " << light->getName() << " " << cam->getName() <<  " " << iteration << std::endl;
        Ogre::Matrix4 view = cam->getViewMatrix();
        Ogre::Matrix4 proj = cam->getProjectionMatrixWithRSDepth();
        static const Ogre::Matrix4 to_uv_space( 0.5,    0,    0,  0.5, 0,   -0.5,    0,  0.5, 0,      0,    1,    0, 0,      0,    0,    1);
        Ogre::Matrix4 view_proj = to_uv_space*proj*view;
        shadow_view_proj[iteration] = view_proj;
        //CVERB << light->getName() << " " << iteration << " " << cam->getViewMatrix() << std::endl;
        //CVERB << light->getName() << " " << iteration << " " << cam->getProjectionMatrixRS() << std::endl;
    }

    virtual void shadowTextureReceiverPreViewProj (Ogre::Light *light, Ogre::Frustum *frustum)
    {
        CVERB << "shadowTextureReceiverPreViewProj: " << light->getName() << " " << frustum->getName() << std::endl;
    }

    //virtual bool sortLightsAffectingFrustum (Ogre::LightList &lightList)
    //{
    //    //CVERB << "sortLightsAffectingFrustum: " << lightList.size() << std::endl;
    //    return false;
    //}

    //virtual void sceneManagerDestroyed (Ogre::SceneManager *)
    //{
    //    //CVERB << "sceneManagerDestroyed" << std::endl;
    //}

} ogre_sm_listener;

// }}}


// {{{ INIT / SHUTDOWN

size_t gfx_init (GfxCallback &cb_)
{
    try {
        gfx_cb = &cb_;

        Ogre::LogManager *lmgr = OGRE_NEW Ogre::LogManager();
        Ogre::Log *ogre_log = OGRE_NEW Ogre::Log("",false,true);
        ogre_log->addListener(&log_listener);
        lmgr->setDefaultLog(ogre_log);
        lmgr->setLogDetail(Ogre::LL_NORMAL);

        ogre_root = OGRE_NEW Ogre::Root("","","");

        octree = OGRE_NEW Ogre::OctreePlugin();
        ogre_root->installPlugin(octree);

        cg = OGRE_NEW Ogre::CgPlugin();
        ogre_root->installPlugin(cg);

        if (d3d9) {
            #ifdef WIN32
            ogre_rs = OGRE_NEW Ogre::D3D9RenderSystem(GetModuleHandle(NULL));
            ogre_rs->setConfigOption("Allow NVPerfHUD","Yes");
            ogre_rs->setConfigOption("Floating-point mode","Consistent");
            ogre_rs->setConfigOption("Video Mode","800 x 600 @ 32-bit colour");
            #endif
        } else {
            ogre_rs = OGRE_NEW Ogre::GLRenderSystem();
            ogre_rs->setConfigOption("RTT Preferred Mode","FBO");
            ogre_rs->setConfigOption("Video Mode","800 x 600");
        }
        ogre_rs->setConfigOption("sRGB Gamma Conversion",use_hwgamma?"Yes":"No");
        ogre_rs->setConfigOption("Full Screen","No");
        ogre_rs->setConfigOption("VSync","Yes");
        ogre_root->setRenderSystem(ogre_rs);

        ogre_root->initialise(true,"Grit Game Window");

        ogre_win = ogre_root->getAutoCreatedWindow();

        size_t winid;
        ogre_win->getCustomAttribute("WINDOW", &winid);
        #ifdef WIN32
        HMODULE mod = GetModuleHandle(NULL);
        HICON icon_big = (HICON)LoadImage(mod, MAKEINTRESOURCE(118), IMAGE_ICON,
                                          0, 0, LR_DEFAULTSIZE|LR_SHARED);
        HICON icon_small = (HICON)LoadImage(mod,MAKEINTRESOURCE(118), IMAGE_ICON,
                                          16, 16, LR_DEFAULTSIZE|LR_SHARED);
        SendMessage((HWND)winid, (UINT)WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) icon_big);
        SendMessage((HWND)winid, (UINT)WM_SETICON, (WPARAM) ICON_SMALL, (LPARAM) icon_small);
        #endif

        ogre_win->setDeactivateOnFocusChange(false);

        Ogre::TextureManager::getSingleton().setVerbose(false);
        Ogre::MeshManager::getSingleton().setVerbose(false);
        Ogre::OverlayManager::getSingleton()
                .addOverlayElementFactory(new HUD::TextListOverlayElementFactory());
        ogre_root->addMovableObjectFactory(new MovableClutterFactory());
        ogre_root->addMovableObjectFactory(new RangedClutterFactory());

        Ogre::MeshManager::getSingleton().setListener(&mesh_serializer_listener);
        Ogre::WindowEventUtilities::addWindowEventListener(ogre_win, &window_event_listener);
        //Ogre::CompositorManager::getSingleton()
        //    .registerCustomCompositionPass(DEFERRED_LIGHTS_CUSTOM_PASS, new DeferredLightsPass());
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(".","FileSystem",RESGRP,true);
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
        
        ftcv = Ogre::ControllerManager::getSingleton().getFrameTimeSource().staticCast<Ogre::FrameTimeControllerValue>();
        ogre_sm = static_cast<Ogre::OctreeSceneManager*>(ogre_root->createSceneManager("OctreeSceneManager"));
        ogre_sm->addListener(&ogre_sm_listener);
        ogre_root_node = ogre_sm->getRootSceneNode();
        ogre_sm->setShadowCasterRenderBackFaces(false);
        ogre_sm->setShadowTextureSelfShadow(true);
        ogre_sun = ogre_sm->createLight("Sun");
        ogre_sun->setType(Ogre::Light::LT_DIRECTIONAL);
        ogre_sky_node = ogre_sm->getSkyCustomNode();

        gfx_pipeline_init();
        gfx_option_init();
        gfx_particle_init();
 
        gfx_env_cube_set("");

        return winid;
    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT2(e.getFullDescription(), "Initialising graphics subsystem");
    }
}

HUD::RootPtr gfx_init_hud (void)
{
    return HUD::RootPtr(new HUD::Root(ogre_win->getWidth(),ogre_win->getHeight()));
}

GfxMaterialType gfx_material_type (const std::string &name)
{
    GFX_MAT_SYNC;
    if (!gfx_material_has_any(name)) GRIT_EXCEPT("Non-existent material: \""+name+"\"");
    GfxBaseMaterial *mat = material_db[name];
    if (dynamic_cast<GfxMaterial*>(mat) != NULL) return GFX_MATERIAL;
    if (dynamic_cast<GfxSkyMaterial*>(mat) != NULL) return GFX_SKY_MATERIAL;
    GRIT_EXCEPT("Internal error: unrecognised kind of material");
    return GFX_MATERIAL; // never get here
}

bool gfx_material_has_any (const std::string &name)
{
    GFX_MAT_SYNC;
    return material_db.find(name) != material_db.end();
}

void gfx_material_add_dependencies (const std::string &name, GfxDiskResource *into)
{
    if (!gfx_material_has_any(name)) GRIT_EXCEPT("Non-existent material: \""+name+"\"");
    material_db[name]->addDependencies(into);
}

void gfx_shutdown (void)
{
    try {
        if (shutting_down) return;
        shutting_down = true;
        delete eye_left;
        delete eye_right;
        env_cube_tex.setNull();
        ftcv.setNull();
        if (ogre_sm && ogre_root) ogre_root->destroySceneManager(ogre_sm);
        if (ogre_root) OGRE_DELETE ogre_root; // internally deletes ogre_rs
        #ifdef NO_PLUGINS
            OGRE_DELETE octree;
            OGRE_DELETE cg;
        #endif
    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT2(e.getFullDescription(), "Shutting down graphics subsystem");
    }

}

// }}}
