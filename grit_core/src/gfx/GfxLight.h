/* Copyright (c) David Cunningham and the Grit Game Engine project 2012
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

class GfxLight;
typedef SharedPtr<GfxLight> GfxLightPtr;

#ifndef GFX_LIGHT_H
#define GFX_LIGHT_H

#include "gfx.h"
#include "GfxBody.h"

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

#endif
