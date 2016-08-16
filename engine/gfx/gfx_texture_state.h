/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
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

struct GfxTextureState;

#ifndef GFX_TEXTURE_STATE_H
#define GFX_TEXTURE_STATE_H

#include <OgreCommon.h>
#include <OgreTextureUnitState.h>

enum GfxTextureAddrMode {
    GFX_AM_WRAP, GFX_AM_MIRROR, GFX_AM_CLAMP, GFX_AM_BORDER
};

static inline Ogre::TextureUnitState::TextureAddressingMode to_ogre(GfxTextureAddrMode m)
{
    switch (m) {
        case GFX_AM_WRAP: return Ogre::TextureUnitState::TAM_WRAP;
        case GFX_AM_MIRROR: return Ogre::TextureUnitState::TAM_MIRROR;
        case GFX_AM_CLAMP: return Ogre::TextureUnitState::TAM_CLAMP;
        case GFX_AM_BORDER: return Ogre::TextureUnitState::TAM_BORDER;
    }
    EXCEPTEX << "Unreachable." << ENDL;
    return Ogre::TextureUnitState::TAM_WRAP;
}

enum GfxTextureFilterMode {
    GFX_FILTER_NONE, GFX_FILTER_POINT, GFX_FILTER_LINEAR, GFX_FILTER_ANISOTROPIC
};

static inline Ogre::FilterOptions to_ogre(GfxTextureFilterMode m)
{
    switch (m) {
        case GFX_FILTER_NONE: return Ogre::FO_NONE;
        case GFX_FILTER_POINT: return Ogre::FO_POINT;
        case GFX_FILTER_LINEAR: return Ogre::FO_LINEAR;
        case GFX_FILTER_ANISOTROPIC: return Ogre::FO_ANISOTROPIC;
    }
    EXCEPTEX << "Unreachable." << ENDL;
    return Ogre::FO_NONE;
}

struct GfxTextureState {
    GfxBaseTextureDiskResource *texture;
    GfxTextureAddrMode modeU, modeV, modeW;
    GfxTextureFilterMode filterMin, filterMax, filterMip;
    int anisotropy;
};

static inline GfxTextureState gfx_texture_state_anisotropic(GfxBaseTextureDiskResource *texture,
                                                            GfxTextureAddrMode mode=GFX_AM_WRAP)
{
    return {
        texture,
        mode, mode, mode,
        GFX_FILTER_ANISOTROPIC, GFX_FILTER_ANISOTROPIC, GFX_FILTER_LINEAR,
        16,
    };
}

static inline GfxTextureState gfx_texture_state_point(GfxBaseTextureDiskResource *texture,
                                                      GfxTextureAddrMode mode=GFX_AM_WRAP)
{
    return {
        texture,
        mode, mode, mode,
        GFX_FILTER_POINT, GFX_FILTER_POINT, GFX_FILTER_NONE,
        0,
    };
}

typedef std::map<std::string, GfxTextureState> GfxTextureStateMap;
        
#endif // GFX_TEXTURE_STATE_H
