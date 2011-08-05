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

/* TODO:
 * sky node
 * clutter / rclutter
 * shaders
 * materials
 * set material of a body
 */

#include "../SharedPtr.h"

// Only things that are referenced from Lua AND can be destroyed (before shutdown) get a SharedPtr
struct GfxCallback;
class GfxBody;
typedef SharedPtr<GfxBody> GfxBodyPtr;
class GfxLight;
typedef SharedPtr<GfxLight> GfxLightPtr;
class GfxMaterial;
struct GfxLastRenderStats;
struct GfxLastFrameStats;
struct GfxRunningFrameStats;
struct GfxPaintColour;
struct GfxParticle;


#ifndef gfx_h
#define gfx_h

#include <string>

#include "HUD.h"

#include "../vect_util.h"
#include "../math_util.h"

#include "GfxDiskResource.h"

struct GfxCallback {
    virtual ~GfxCallback (void) { }
    virtual void clickedClose (void) = 0;
    virtual void windowResized (int width, int height) = 0;
    virtual void messageLogged (const std::string &msg) = 0;
};

size_t gfx_init (GfxCallback &cb);

void gfx_render (float elapsed, const Vector3 &cam_pos, const Quaternion &cam_dir);
enum GfxBoolOption {
    GFX_AUTOUPDATE,
    GFX_SHADOW_RECEIVE,
    GFX_SHADOW_CAST,
    GFX_VSYNC,
    GFX_FULLSCREEN,
    GFX_FOG,
    GFX_DEFERRED,
    GFX_WIREFRAME,
    GFX_ANAGLYPH,
    GFX_CROSS_EYE,
    GFX_SHADOW_SIMPLE_OPTIMAL_ADJUST,
    GFX_SHADOW_AGGRESSIVE_FOCUS_REGION,
    GFX_SKY
};

enum GfxIntOption {
    GFX_FULLSCREEN_WIDTH,
    GFX_FULLSCREEN_HEIGHT,
    GFX_SHADOW_RES,
    GFX_RAM
};

enum GfxFloatOption {
    GFX_FOV,
    GFX_NEAR_CLIP,
    GFX_FAR_CLIP,
    GFX_EYE_SEPARATION, // FOR 'real' 3d
    GFX_MONITOR_HEIGHT, // FOR 'real' 3d
    GFX_MONITOR_EYE_DISTANCE, // FOR 'real' 3d
    GFX_MIN_PERCEIVED_DEPTH, // FOR 'real' 3d -- distance from eyes of the front clip plane
    GFX_MAX_PERCEIVED_DEPTH, // FOR 'real' 3d -- distance from eyes of the rear clip plane
    GFX_SHADOW_START,
    GFX_SHADOW_END0,
    GFX_SHADOW_END1,
    GFX_SHADOW_END2,
    GFX_SHADOW_OPTIMAL_ADJUST0,
    GFX_SHADOW_OPTIMAL_ADJUST1,
    GFX_SHADOW_OPTIMAL_ADJUST2,
    GFX_SHADOW_LIGHT_DIRECTION_THRESHOLD,
    GFX_SHADOW_PADDING,
    GFX_ANAGLYPH_LEFT_RED_MASK,
    GFX_ANAGLYPH_LEFT_GREEN_MASK,
    GFX_ANAGLYPH_LEFT_BLUE_MASK,
    GFX_ANAGLYPH_RIGHT_RED_MASK,
    GFX_ANAGLYPH_RIGHT_GREEN_MASK,
    GFX_ANAGLYPH_RIGHT_BLUE_MASK,
    GFX_ANAGLYPH_DESATURATION
};

std::string gfx_option_to_string (GfxBoolOption o);
std::string gfx_option_to_string (GfxIntOption o);
std::string gfx_option_to_string (GfxFloatOption o);

// set's t to either 0,1,2 and fills in the approriate argument
void gfx_option_from_string (const std::string &s,
                             int &t,
                             GfxBoolOption &o0,
                             GfxIntOption &o1,
                             GfxFloatOption &o2);

void gfx_option (GfxBoolOption o, bool v);
bool gfx_option (GfxBoolOption o);
void gfx_option (GfxIntOption o, int v);
int gfx_option (GfxIntOption o);
void gfx_option (GfxFloatOption o, float v);
float gfx_option (GfxFloatOption o);

class GfxParticleSystem;

struct GfxParticle {
    GfxParticleSystem *sys;
    void *internal;

    GfxParticle (void) { }

    GfxParticle (GfxParticleSystem *sys);

    GfxParticle &operator= (const GfxParticle &other);
    
    void setPosition (const Vector3 &v);
    void setAmbient (const Vector3 &v);
    void setAlpha (float v);
    void setAngle (float v);
    void setUV (float u1, float v1, float u2, float v2);
    void setDefaultUV (void);
    void setWidth (float v);
    void setHeight (float v);
    void setDepth (float v);

    void release (void);
};

enum GfxParticleSceneBlend { GFX_PARTICLE_OPAQUE, GFX_PARTICLE_ALPHA, GFX_PARTICLE_ADD };

void gfx_particle_define (const std::string &pname, const std::string &tex_name,
                          GfxParticleSceneBlend blend, float alpha_rej, bool emissive);
GfxParticle gfx_particle_emit (const std::string &pname);
    
typedef std::vector<GfxMaterial*> GfxMaterials;

enum GfxMaterialSceneBlend { GFX_MATERIAL_OPAQUE, GFX_MATERIAL_ALPHA, GFX_MATERIAL_ALPHA_DEPTH };

enum GfxMaterialSpecularMode {
    GFX_MATERIAL_SPEC_NONE,
    GFX_MATERIAL_SPEC_ADJUSTED_DIFFUSE_COLOUR,
    GFX_MATERIAL_SPEC_DIFFUSE_ALPHA,
    GFX_MATERIAL_SPEC_MAP,
    GFX_MATERIAL_SPEC_MAP_WITH_GLOSS
};

extern boost::recursive_mutex gfx_material_lock;
#define GFX_MAT_SYNC boost::recursive_mutex::scoped_lock _scoped_lock(gfx_material_lock)


// textures and texture settings
class GfxMaterialTextureBlendUnit {

    public:

    GfxMaterialTextureBlendUnit (void)
      : specularDiffuseBrightness(0), specularDiffuseContrast(0), specularDiffuseSaturation(0),
        textureAnimationX(1), textureAnimationY(1),
        textureScaleX(1), textureScaleY(1)
    {
        
    }

    private: std::string diffuseMap;
    public: std::string getDiffuseMap (void) const { GFX_MAT_SYNC; return diffuseMap; }
    public: void setDiffuseMap (const std::string &v) { GFX_MAT_SYNC; diffuseMap = v; }

    private: std::string normalMap;
    public: std::string getNormalMap (void) const { GFX_MAT_SYNC; return normalMap; }
    public: void setNormalMap (const std::string &v) { GFX_MAT_SYNC; normalMap = v; }

    private: std::string specularMap;
    public: std::string getSpecularMap (void) const { GFX_MAT_SYNC; return specularMap; }
    public: void setSpecularMap (const std::string &v) { GFX_MAT_SYNC; specularMap = v; }

    private: float specularDiffuseBrightness;
    public: float getSpecularDiffuseBrightness (void) const { GFX_MAT_SYNC; return specularDiffuseBrightness; }
    public: void setSpecularDiffuseBrightness (float v) { GFX_MAT_SYNC; specularDiffuseBrightness = v; }

    private: float specularDiffuseContrast;
    public: float getSpecularDiffuseContrast (void) const { GFX_MAT_SYNC; return specularDiffuseContrast; }
    public: void setSpecularDiffuseContrast (float v) { GFX_MAT_SYNC; specularDiffuseContrast = v; }

    private: float specularDiffuseSaturation;
    public: float getSpecularDiffuseSaturation (void) const { GFX_MAT_SYNC; return specularDiffuseSaturation; }
    public: void setSpecularDiffuseSaturation (float v) { GFX_MAT_SYNC; specularDiffuseSaturation = v; }

    private: float textureAnimationX;
    public: float getTextureAnimationX (void) const { GFX_MAT_SYNC; return textureAnimationX; }
    public: void setTextureAnimationX (float v) { GFX_MAT_SYNC; textureAnimationX = v; }

    private: float textureAnimationY;
    public: float getTextureAnimationY (void) const { GFX_MAT_SYNC; return textureAnimationY; }
    public: void setTextureAnimationY (float v) { GFX_MAT_SYNC; textureAnimationY = v; }

    private: float textureScaleX;
    public: float getTextureScaleX (void) const { GFX_MAT_SYNC; return textureScaleX; }
    public: void setTextureScaleX (float v) { GFX_MAT_SYNC; textureScaleX = v; }

    private: float textureScaleY;
    public: float getTextureScaleY (void) const { GFX_MAT_SYNC; return textureScaleY; }
    public: void setTextureScaleY (float v) { GFX_MAT_SYNC; textureScaleY = v; }

    public:

    bool hasDiffuseMap (void) { GFX_MAT_SYNC; return diffuseMap.length() > 0; }

    bool hasNormalMap (void) { GFX_MAT_SYNC; return normalMap.length() > 0; }

};

enum GfxMaterialPaintMode {
    GFX_MATERIAL_PAINT_NONE,
    GFX_MATERIAL_PAINT_1,
    GFX_MATERIAL_PAINT_2,
    GFX_MATERIAL_PAINT_3,
    GFX_MATERIAL_PAINT_4,
    GFX_MATERIAL_PAINT_MAP
};

class GfxMaterial {
    public: // hack
    Ogre::MaterialPtr regularMat;     // no suffix
    Ogre::MaterialPtr fadingMat;      // ' can be NULL
    Ogre::MaterialPtr emissiveMat;
    //Ogre::MaterialPtr shadowMat;      // ! can be simply a link to the default
    //Ogre::MaterialPtr worldMat;       // & 
    //Ogre::MaterialPtr worldShadowMat; // % can be simply a link to the default


    private: GfxMaterial (const std::string &name);

    private: float alpha;
    public: float getAlpha (void) const { GFX_MAT_SYNC; return alpha; }
    public: void setAlpha (float v) { GFX_MAT_SYNC; alpha = v; }

    private: GfxMaterialSceneBlend sceneBlend;
    public: GfxMaterialSceneBlend getSceneBlend (void) const { GFX_MAT_SYNC; return sceneBlend; }
    public: void setSceneBlend (GfxMaterialSceneBlend v);

    private: bool stipple;
    public: bool getStipple (void) const { GFX_MAT_SYNC; return stipple; }
    public: void setStipple (bool v);

    private: Vector3 emissiveColour;
    public: Vector3 getEmissiveColour (void) const { GFX_MAT_SYNC; return emissiveColour; }
    public: void setEmissiveColour (const Vector3 &v);


    private: std::string emissiveMap;
    public: std::string getEmissiveMap (void) const { GFX_MAT_SYNC; return emissiveMap; }
    public: void setEmissiveMap (const std::string &v) { GFX_MAT_SYNC; emissiveMap = v; }

    private: std::string paintMap;
    public: std::string getPaintMap (void) const { GFX_MAT_SYNC; return paintMap; }
    public: void setPaintMap (const std::string &v) { GFX_MAT_SYNC; paintMap = v; }

    private: GfxMaterialPaintMode paintMode;
    public: GfxMaterialPaintMode getPaintMode (void) const { GFX_MAT_SYNC; return paintMode; }
    public: void setPaintMode (const GfxMaterialPaintMode &v) { GFX_MAT_SYNC; paintMode = v; }

    private: bool paintByDiffuseAlpha;
    public: bool getPaintByDiffuseAlpha (void) const { GFX_MAT_SYNC; return paintByDiffuseAlpha; }
    public: void setPaintByDiffuseAlpha (const bool &v) { GFX_MAT_SYNC; paintByDiffuseAlpha = v; }

    private: GfxMaterialSpecularMode specularMode;
    public: GfxMaterialSpecularMode getSpecularMode (void) const { GFX_MAT_SYNC; return specularMode; }
    public: void setSpecularMode (const GfxMaterialSpecularMode &v) { GFX_MAT_SYNC; specularMode = v; }

    private: unsigned numTextureBlends;
    public: unsigned getNumTextureBlends (void) const { GFX_MAT_SYNC; return numTextureBlends; }
    public: void setNumTextureBlends (const unsigned &v) { GFX_MAT_SYNC; numTextureBlends = v; }

    public: GfxMaterialTextureBlendUnit texBlends[4];

    public:

    bool hasDiffuseMap() {
        GFX_MAT_SYNC;
        for (unsigned i=0 ; i<numTextureBlends ; ++i) {
            if (texBlends[i].hasDiffuseMap()) return true;
        }
        return false;
    }

    bool hasNormalMap() {
        GFX_MAT_SYNC;
        for (unsigned i=0 ; i<numTextureBlends ; ++i) {
            if (texBlends[i].hasNormalMap()) return true;
        }
        return false;
    }

    bool hasSpecularMap (void) {
        GFX_MAT_SYNC;
        return specularMode == GFX_MATERIAL_SPEC_MAP || specularMode == GFX_MATERIAL_SPEC_MAP_WITH_GLOSS;
    }

    bool hasEmissiveMap() {
        GFX_MAT_SYNC;
        return emissiveMap.length() > 0;
    }

    bool hasPaintMap() {
        GFX_MAT_SYNC;
        return paintMode == GFX_MATERIAL_PAINT_MAP;
    }

    const std::string name;

    friend GfxMaterial *gfx_material_add(const std::string &);
    friend class GfxBody;
};

GfxMaterial *gfx_material_add (const std::string &name);

GfxMaterial *gfx_material_add_or_get (const std::string &name);

GfxMaterial *gfx_material_get (const std::string &name);

bool gfx_material_has (const std::string &name);

struct GfxPaintColour {
    Vector3 diff, spec;
    float met; // metallic paint (0 -> 1)
};


// this should rarely need to be used by users of this API
class GfxNode {
    protected:
    static const std::string className;
    Vector3 pos; Quaternion quat; Vector3 scl;
    GfxBodyPtr par;
    Ogre::MeshPtr mesh;
    public: // HACK
    Ogre::SceneNode *node;
    protected:
    bool dead;

    GfxNode (const GfxBodyPtr &par_);
    virtual ~GfxNode ();

    void notifyParentDead (void);
    void ensureNotChildOf (GfxBody *leaf) const;

    public:

    const GfxBodyPtr &getParent (void) const;
    virtual void setParent (const GfxBodyPtr &par_);

    Vector3 transformPositionParent (const Vector3 &v);
    Quaternion transformOrientationParent (const Quaternion &v);
    Vector3 transformScaleParent (const Vector3 &v);
    Vector3 transformPosition (const Vector3 &v);
    Quaternion transformOrientation (const Quaternion &v);
    Vector3 transformScale (const Vector3 &v);

    const Vector3 &getLocalPosition (void);
    void setLocalPosition (const Vector3 &p);
    Vector3 getWorldPosition (void);

    const Quaternion &getLocalOrientation (void);
    void setLocalOrientation (const Quaternion &q);
    Quaternion getWorldOrientation (void);

    const Vector3 &getLocalScale (void);
    void setLocalScale (const Vector3 &s);
    Vector3 getWorldScale (void);

    virtual void destroy (void);
    virtual bool destroyed (void) const { return dead; }

    friend class GfxBody; // otherwise it cannot access our protected stuff
};

typedef std::map<std::string, std::string> GfxStringMap;
const GfxStringMap gfx_empty_string_map;

class GfxBody : public GfxNode, public fast_erase_index {
    protected:
    static const std::string className;
    std::vector<GfxNode*> children; // caution!
    public: // HACK
    Ogre::Entity *ent;
    Ogre::Entity *entEmissive;
    protected:
    float fade;
    GfxMaterials materials;
    std::vector<bool> emissiveEnabled;
    GfxPaintColour colours[4];
    bool enabled;
    std::vector<bool> manualBones;
    GfxStringMap initialMaterialMap;

    GfxBody (GfxDiskResource *gdr, const GfxStringMap &sm, const GfxBodyPtr &par_);
    GfxBody (const GfxBodyPtr &par_);
    ~GfxBody ();


    public:
    static GfxBodyPtr make (const std::string &mesh_name,
                            const GfxStringMap &sm=gfx_empty_string_map,
                            const GfxBodyPtr &par_=GfxBodyPtr(NULL));

    static GfxBodyPtr make (const GfxBodyPtr &par_=GfxBodyPtr(NULL))
    { return GfxBodyPtr(new GfxBody(par_)); }
    
    GfxMaterial *getMaterial (unsigned i);
    const std::string &getOriginalMaterialName (unsigned i);
    unsigned getSubMeshByOriginalMaterialName (const std::string &n);
    void setMaterial (unsigned i, GfxMaterial *m);
    bool getEmissiveEnabled (unsigned i);
    void setEmissiveEnabled (unsigned i, bool v);
    unsigned getNumSubMeshes (void) { return materials.size(); }

    void notifyLostChild (GfxNode *child);
    void notifyGainedChild (GfxNode *child);
    void setParent (const GfxBodyPtr &par_);

    protected:
    void updateEntEmissive (void);
    void updateVisibility (void);
    void updateMaterials (void);
    void updateBones (void);
    void checkBone (unsigned n);
    public:
    void updateProperties (void);
    void reinitialise (void);

    unsigned getBatches (void) const;
    unsigned getBatchesWithChildren (void) const;

    unsigned getTriangles (void) const;
    unsigned getTrianglesWithChildren (void) const;

    float getFade (void);
    void setFade (float f);

    bool getCastShadows (void);
    void setCastShadows (bool v);

    GfxPaintColour getPaintColour (int i);
    void setPaintColour (int i, const GfxPaintColour &c);

    unsigned getNumBones (void);
    unsigned getBoneId (const std::string name);
    const std::string &getBoneName (unsigned n);

    bool getBoneManuallyControlled (unsigned n);
    void setBoneManuallyControlled (unsigned n, bool v);
    void setAllBonesManuallyControlled (bool v);

    Vector3 getBoneInitialPosition (unsigned n);
    Vector3 getBoneWorldPosition (unsigned n);
    Vector3 getBoneLocalPosition (unsigned n);
    Quaternion getBoneInitialOrientation (unsigned n);
    Quaternion getBoneWorldOrientation (unsigned n);
    Quaternion getBoneLocalOrientation (unsigned n);

    void setBoneLocalPosition (unsigned n, const Vector3 &v);
    void setBoneLocalOrientation (unsigned n, const Quaternion &v);

    bool isEnabled (void);
    void setEnabled (bool v);

    void destroy (void);

    bool hasGraphics (void) const { return ent!=NULL; }


    friend class SharedPtr<GfxBody>;
    friend void gfx_reload_mesh (const std::string &name);
};

class GfxLight : public GfxNode, public fast_erase_index {
    protected:
    static const std::string className;
    bool enabled;
    float fade;
    Vector3 coronaLocalPos;
    Vector3 coronaPos;
    float coronaSize;
    Vector3 coronaColour;
    Vector3 diffuse;
    Vector3 specular;
    GfxParticle corona;
    Quaternion aim;
    public: // HACK
    Ogre::Light *light;
    protected:

    GfxLight (const GfxBodyPtr &par_);
    ~GfxLight ();

    void updateVisibility (void);

    public:
    static GfxLightPtr make (const GfxBodyPtr &par_=GfxBodyPtr(NULL))
    { return GfxLightPtr(new GfxLight(par_)); }
    
    Vector3 getDiffuseColour (void);
    Vector3 getSpecularColour (void);
    void setDiffuseColour (const Vector3 &v);
    void setSpecularColour (const Vector3 &v);
    float getRange (void);
    void setRange (float v);
    Degree getInnerAngle (void);
    void setInnerAngle (Degree v);
    Degree getOuterAngle (void);
    void setOuterAngle (Degree v);

    bool isEnabled (void);
    void setEnabled (bool v);

    float getFade (void);
    void setFade (float f);

    void updateCorona (const Vector3 &cam_pos);

    Vector3 getCoronaLocalPosition (void);
    void setCoronaLocalPosition (const Vector3 &v);

    float getCoronaSize (void);
    void setCoronaSize (float v);
    Vector3 getCoronaColour (void);
    void setCoronaColour (const Vector3 &v);

    void destroy (void);

    friend class SharedPtr<GfxLight>;
};


Vector3 gfx_sun_get_diffuse (void);
void gfx_sun_set_diffuse (const Vector3 &v);
Vector3 gfx_sun_get_specular (void);
void gfx_sun_set_specular (const Vector3 &v);
Vector3 gfx_sun_get_direction (void);
void gfx_sun_set_direction (const Vector3 &v);

Vector3 gfx_get_scene_ambient (void);
void gfx_set_scene_ambient (const Vector3 &v);

Vector3 gfx_fog_get_colour (void);
void gfx_fog_set_colour (const Vector3 &v);
float gfx_fog_get_density (void);
void gfx_fog_set_density (float v);

Quaternion gfx_get_celestial_orientation (void);
void gfx_set_celestial_orientation (const Quaternion &v);

void gfx_screenshot (const std::string &filename);

struct GfxLastRenderStats {
    float batches;
    float triangles;
    float micros;
    GfxLastRenderStats (const GfxLastRenderStats &o)
        : batches(o.batches), triangles(o.triangles), micros(o.micros) { }
    GfxLastRenderStats (void) : batches(0), triangles(0), micros(0) { }
    GfxLastRenderStats &operator+= (const GfxLastRenderStats &o) {
        batches += o.batches; triangles += o.triangles; micros += o.micros;
        return *this;
    }
    GfxLastRenderStats &operator/= (float time) {
        batches /= time; triangles /= time; micros /= time;
        return *this;
    }
};

struct GfxLastFrameStats {
    GfxLastRenderStats shadow[3];
    GfxLastRenderStats left;
    GfxLastRenderStats right;
};

struct GfxRunningFrameStats {
    GfxLastFrameStats min;
    GfxLastFrameStats avg;
    GfxLastFrameStats max;
};

GfxLastFrameStats gfx_last_frame_stats (void);
GfxRunningFrameStats gfx_running_frame_stats (void);

void gfx_reload_mesh (const std::string &name);
void gfx_reload_texture (const std::string &name);

HUD::RootPtr gfx_init_hud (void);

void gfx_shutdown (void);

// FIXME: everything below here is a horrible hack
#include <OgreRoot.h>
#include <OgreOctreeSceneManager.h>
#include <OgreRenderWindow.h>
extern Ogre::Root *ogre_root; // FIXME: hack
extern Ogre::OctreeSceneManager *ogre_sm; // FIXME: hack
extern Ogre::RenderWindow *ogre_win; // FIXME: hack


static inline Ogre::Vector3 to_ogre (const Vector3 &v)
{ return Ogre::Vector3(v.x,v.y,v.z); }
static inline Ogre::Quaternion to_ogre (const Quaternion &v)
{ return Ogre::Quaternion(v.w,v.x,v.y,v.z); }
static inline Ogre::ColourValue to_ogre_cv (const Vector3 &v)
{ return Ogre::ColourValue(v.x,v.y,v.z); }
static inline Ogre::Degree to_ogre (const Degree &v)
{ return Ogre::Degree(v.inDegrees()); }
static inline Ogre::Radian to_ogre (const Radian &v)
{ return Ogre::Degree(v.inRadians()); }

static inline Vector3 from_ogre (const Ogre::Vector3 &v)
{ return Vector3(v.x,v.y,v.z); }
static inline Quaternion from_ogre (const Ogre::Quaternion &v)
{ return Quaternion(v.w,v.x,v.y,v.z); }
static inline Vector3 from_ogre_cv (const Ogre::ColourValue &v)
{ return Vector3(v.r, v.g, v.b); }
static inline Degree from_ogre (const Ogre::Degree &v)
{ return Degree(v.valueDegrees()); }
static inline Radian from_ogre (const Ogre::Radian &v)
{ return Radian(v.valueRadians()); }

#endif 
