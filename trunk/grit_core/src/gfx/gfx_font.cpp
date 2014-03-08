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

#include "gfx_font.h"

typedef std::map<std::string, GfxFont *> FontMap;
FontMap db;

Vector2 GfxFont::getTextureDimensions (void) {
    Ogre::TexturePtr ptr = texture->getOgreTexturePtr();
    ptr->load(); // push to GPU, otherwise width and height are wrong
    return Vector2(ptr->getWidth(), ptr->getHeight());
}

bool gfx_font_has (const std::string &name)
{
    FontMap::iterator it = db.find(name);
    if (it == db.end()) return false;
    return true;
}

std::vector<GfxFont*> gfx_font_list (void)
{
    std::vector<GfxFont*> r;
    for (FontMap::const_iterator i=db.begin(),i_=db.end() ; i!=i_ ; ++i) {
        r.push_back(i->second);
    }
    return r;
}

GfxFont *gfx_font_get (const std::string &name)
{
    if (!gfx_font_has(name)) return NULL;
    return db[name];
}

GfxFont *gfx_font_make (const std::string &name, GfxTextureDiskResource *dr, unsigned long height)
{
    GfxFont *f = gfx_font_get(name);
    if (f == NULL) {
        f = new GfxFont(name, dr, height);
        db[name] = f;
    } else {
        f->setTexture(dr);
        f->clearCodePoints();
    }
    return f;
}

unsigned long gfx_font_num (void)
{
    return db.size();
}
