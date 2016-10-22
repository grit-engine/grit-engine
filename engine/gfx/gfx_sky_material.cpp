/* Copyright (c) The Grit Game Engine authors 2016
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

#include <string>
#include <map>

#include <centralised_log.h>

#include "gfx_internal.h"
#include "gfx_sky_material.h"
#include "gfx_sky_body.h"


GfxSkyMaterial *gfx_sky_material_add (const std::string &name)
{
    GFX_MAT_SYNC;
    if (gfx_sky_material_has(name)) GRIT_EXCEPT("Material already exists: \""+name+"\"");
    GfxSkyMaterial *r = new GfxSkyMaterial(name);
    material_db[name] = r;
    return r;
}

GfxSkyMaterial *gfx_sky_material_add_or_get (const std::string &name)
{
    GFX_MAT_SYNC;
    if (gfx_material_has_any(name)) {
        GfxSkyMaterial *mat = dynamic_cast<GfxSkyMaterial*>(material_db[name]);
        if (mat==NULL) GRIT_EXCEPT("Material already exists but is the wrong kind: \""+name+"\"");
        return mat;
    }
    return gfx_sky_material_add(name);
}

GfxSkyMaterial *gfx_sky_material_get (const std::string &name)
{
    GFX_MAT_SYNC;
    if (!gfx_sky_material_has(name)) GRIT_EXCEPT("Sky material does not exist: \""+name+"\"");
    GfxSkyMaterial *mat = dynamic_cast<GfxSkyMaterial*>(material_db[name]);
    if (mat==NULL) GRIT_EXCEPT("Wrong kind of material: \""+name+"\"");
    return mat;
}

bool gfx_sky_material_has (const std::string &name)
{
    GFX_MAT_SYNC;
    GfxMaterialDB::iterator it = material_db.find(name);
    if (it == material_db.end()) return false;
    return dynamic_cast<GfxSkyMaterial*>(it->second) != NULL;
}

GfxSkyMaterial::GfxSkyMaterial (const std::string &name)
  : GfxBaseMaterial(name, gfx_shader_get("/system/SkyDefault")),
    sceneBlend(GFX_SKY_MATERIAL_OPAQUE)
{
}

void gfx_sky_material_init (void)
{
    std::string vs = "out.position = transform_to_world(vert.position.xyz);\n";
    std::string fs = "var c = pma_decode(sample(mat.emissiveMap, vert.coord0.xy))\n"
                     "             * Float4(1, 1, 1, mat.alphaMask);\n"
                     "if (c.a <= mat.alphaRejectThreshold) discard;\n"
                     "out.colour = gamma_decode(c.rgb) * mat.emissiveMask;\n"
                     "out.alpha = c.a;\n";

    gfx_shader_make_or_reset("/system/SkyDefault", vs, "", fs, {
        { "alphaMask", GfxGslParam::float1(1.0f) },
        { "alphaRejectThreshold", GfxGslParam::float1(-1.0f) },
        { "emissiveMap", GfxGslParam(GFX_GSL_FLOAT_TEXTURE2, 1, 1, 1, 1) },
        { "emissiveMask", GfxGslParam::float3(1, 1, 1) },
    }, false);

    gfx_sky_material_add("/system/SkyDefault");
}
