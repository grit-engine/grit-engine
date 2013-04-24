/* Copyright (c) David Cunningham and the Grit Game Engine project 2013
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

#include <vector>

class GfxFont;
typedef std::vector<GfxFont*> GfxFonts;

#ifndef GFXFONT_H
#define GFXFONT_H

#include <map>

#include "../BackgroundLoader.h"
#include "../main.h"
#include "../math_util.h"

#include "GfxDiskResource.h"

class GfxFont {
    public:
    typedef unsigned long codepoint_t;
    struct CharRect {
        float u1, v1, u2, v2;
    };
    typedef std::map<codepoint_t, CharRect> CharRectMap;
    const std::string name;
    private:
    GfxTextureDiskResource *texture;
    float height;
    CharRectMap coords;
    public:
    GfxFont (const std::string &name, GfxTextureDiskResource *tex, float height)
      : name(name), texture(tex), height(height)
    {
        texture->increment();
        if (!texture->isLoaded()) texture->load();
    }
    ~GfxFont (void)
    {
        texture->decrement();
        bgl->finishedWith(texture);
    }
    float getHeight (void) { return height; }
    void setHeight (float v) { height = v; }
    GfxTextureDiskResource *getTexture (void) {
        return texture;
    }
    void setTexture (GfxTextureDiskResource *tex) {
        APP_ASSERT(tex != NULL);
        tex->increment();
        texture->decrement();
        bgl->finishedWith(texture);
        if (!tex->isLoaded()) tex->load();
        texture = tex;
    }
    bool hasCodePoint (codepoint_t cp) const {
        CharRectMap::const_iterator it = coords.find(cp);
        return it != coords.end();
    }
    void setCodePoint (codepoint_t cp, const CharRect &r) {
        coords[cp] = r;
    }
    bool getCodePointOrFail (codepoint_t cp, CharRect &r) const {
        CharRectMap::const_iterator it = coords.find(cp);
        if (it == coords.end()) return false;
        r = it->second;
        return true;
    }
    Vector2 getTextureDimensions (void);
    void clearCodePoints (void) { coords.clear(); }
};

bool gfx_font_has (const std::string &name);
GfxFont *gfx_font_get (const std::string &name);
GfxFont *gfx_font_make (const std::string &name, GfxTextureDiskResource *dr, float height);
unsigned long gfx_font_num (void);

#endif
