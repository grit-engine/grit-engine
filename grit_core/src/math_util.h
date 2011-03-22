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

#ifndef MATH_UTIL_H
#define MATH_UTIL_H

struct Radian;
struct Degree;
struct Quaternion;
struct Vector3;

#include <cmath>
#include <cfloat>

#include "CentralisedLog.h"

#ifdef WIN32
#define my_isnan _isnan
#else
#define my_isnan isnan
#endif

#ifndef M_PI
#define M_PI 3.1415926535897932385f
#endif

// {{{ Degree & Radian

struct Radian {
    explicit Radian (float f_) : f(f_) { }
    Radian (void) { }
    Radian (const Degree &d);
    Radian (const Radian &r) : f(r.f) { }
    float inDegrees (void) const { return f / M_PI * 180; }
    float inRadians (void) const { return f; }
    Radian &operator = (const Radian &o) { f = o.inRadians(); return *this; }
    protected:
    float f;
};

struct Degree {
    explicit Degree (float f_) : f(f_) { }
    Degree (void) { }
    Degree (const Radian &r) : f(r.inDegrees()) { }
    Degree (const Degree &r) : f(r.f) { }
    float inDegrees (void) const { return f; }
    float inRadians (void) const { return f * M_PI / 180; }
    Degree &operator = (const Degree &o) { f = o.inDegrees(); return *this; }
    protected:
    float f;
};

inline Radian::Radian (const Degree &d) : f(d.inRadians()) { }

inline float gritcos (Radian r) { return cosf(r.inRadians()); }
inline float gritsin (Radian r) { return sinf(r.inRadians()); }
inline Radian gritacos (float x) { return Radian(acosf(x)); }
inline Radian gritasin (float x) { return Radian(asinf(x)); }

// }}}




// {{{ Quaternion

struct Quaternion {

    float w, x, y, z;

    Quaternion () { }
    Quaternion (float w_, float x_, float y_, float z_) : w(w_), x(x_), y(y_), z(z_) { }
    Quaternion (const Radian& a, const Vector3& axis);

    Quaternion& operator= (const Quaternion& o)
    { w = o.w; x = o.x; y = o.y; z = o.z; return *this; }
    friend Quaternion operator+ (const Quaternion& a, const Quaternion& b)
    { return Quaternion(a.w+b.w, a.x+b.x, a.y+b.y, a.z+b.z); }
    friend Quaternion operator- (const Quaternion& a, const Quaternion& b)
    { return Quaternion(a.w-b.w, a.x-b.x, a.y-b.y, a.z-b.z); }
    friend Quaternion operator* (const Quaternion& a, const Quaternion& b)
    {
        return Quaternion (a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z,
                           a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
                           a.w*b.y + a.y*b.w + a.z*b.x - a.x*b.z,
                           a.w*b.z + a.z*b.w + a.x*b.y - a.y*b.x);
    }
    friend Quaternion operator* (float a, const Quaternion& b)
    { return Quaternion(a*b.w, a*b.x, a*b.y, a*b.z); }
    friend Quaternion operator* (const Quaternion& a, float b)
    { return Quaternion(b*a.w, b*a.x, b*a.y, b*a.z); }
    friend Quaternion operator/ (float a, const Quaternion& b)
    { return Quaternion(a/b.w, a/b.x, a/b.y, a/b.z); }
    friend Quaternion operator/ (const Quaternion& a, float b)
    { return Quaternion(a.w/b, a.x/b, a.y/b, a.z/b); }
    friend Quaternion &operator*= (Quaternion& a, float b)
    { a = a * b; return a; }
    friend Quaternion &operator/= (Quaternion& a, float b)
    { a = a / b; return a; }
    friend Quaternion &operator+= (Quaternion& a, Quaternion &b)
    { a = a + b; return a; }
    friend Quaternion &operator*= (Quaternion& a, Quaternion &b)
    { a = a * b; return a; }
        

    Quaternion operator- () const { return Quaternion(-w,-x,-y,-z); }

    friend bool operator== (const Quaternion &a, const Quaternion &b)
    { return a.w==b.w && a.x==b.x && a.y==b.y && a.z==b.z; }

    friend bool operator!= (const Quaternion &a, const Quaternion &b) { return !(a == b); }

    float dot (const Quaternion &o) const { return w*o.w + x*o.x + y*o.y + z*o.z; }
    float length2 () const { return dot(*this); }
    float length () const { return ::sqrtf(length2()); }

    float normalise (void) { float l = this->length(); *this /= l; return l; }
    Quaternion normalisedCopy (void) const { Quaternion q = *this; q.normalise(); return q; }


    Quaternion unitInverse (void) const
    { return Quaternion(w,-x,-y,-z); }
    Quaternion inverse (void) const
    { return this->normalisedCopy().unitInverse(); }

    bool isNaN() const { return my_isnan(w) || my_isnan(x) || my_isnan(y) || my_isnan(z); }

    friend std::ostream& operator << (std::ostream& o, const Quaternion& q)
    {
        o << "Quaternion(" << q.w << ", " << q.x << ", " << q.y << ", " << q.z << ")";
        return o;
    }

};

// }}}


// {{{ Vector3

struct Vector3 {
    float x, y, z;

    Vector3 () { }
    Vector3 (float x_, float y_, float z_) : x(x_), y(y_), z(z_) { }

    Vector3& operator = (const Vector3& o)
    {
        x = o.x; y = o.y; z = o.z;
        return *this;
    }

    friend bool operator == (const Vector3 &a, const Vector3& b)
    { return a.x==b.x && a.y==b.y && a.z==b.z; }
    friend bool operator != (const Vector3 &a, const Vector3& b)
    { return ! (a==b); }
    friend bool operator < (const Vector3 &a, const Vector3& b)
    { return a.x<b.x && a.y<b.y && a.z<b.z; }
    friend bool operator > (const Vector3 &a, const Vector3& b)
    { return a.x>b.x && a.y>b.y && a.z>b.z; }
    friend bool operator <= (const Vector3 &a, const Vector3& b)
    { return a.x<=b.x && a.y<=b.y && a.z<=b.z; }
    friend bool operator >= (const Vector3 &a, const Vector3& b)
    { return a.x>=b.x && a.y>=b.y && a.z>=b.z; }

    friend Vector3 operator + (const Vector3 &a, const Vector3& b)
    { return Vector3(a.x+b.x, a.y+b.y, a.z+b.z); }
    friend Vector3 operator - (const Vector3 &a, const Vector3& b)
    { return Vector3(a.x-b.x, a.y-b.y, a.z-b.z); }
    friend Vector3 operator * (const Vector3 &a, const Vector3& b)
    { return Vector3(a.x*b.x, a.y*b.y, a.z*b.z); }
    friend Vector3 operator / (const Vector3 &a, const Vector3& b)
    { return Vector3(a.x/b.x, a.y/b.y, a.z/b.z); }

    friend Vector3 operator * (const float a, const Vector3& b)
    { return Vector3(a*b.x, a*b.y, a*b.z); }
    friend Vector3 operator * (const Vector3 &b, const float a)
    { return Vector3(a*b.x, a*b.y, a*b.z); }

    friend Vector3 operator / (const float a, const Vector3& b)
    { return Vector3(a/b.x, a/b.y, a/b.z); }
    friend Vector3 operator / (const Vector3 &a, const float b)
    { return a * (1/b); }

    friend const Vector3 &operator + (const Vector3& a)
    { return a; }
    friend Vector3 operator - (const Vector3& a)
    { return Vector3(-a.x, -a.y, -a.z); }

    friend Vector3 &operator += (Vector3 &a, const Vector3& b)
    { a.x+=b.x, a.y+=b.y, a.z+=b.z; return a; }
    friend Vector3 &operator += (Vector3 &a, const float b)
    { a.x+=b, a.y+=b, a.z+=b; return a; }
    friend Vector3 &operator -= (Vector3 &a, const Vector3& b)
    { a.x-=b.x, a.y-=b.y, a.z-=b.z; return a; }
    friend Vector3 &operator -= (Vector3 &a, const float b)
    { a.x-=b, a.y-=b, a.z-=b; return a; }
    friend Vector3 &operator *= (Vector3 &a, const Vector3& b)
    { a.x*=b.x, a.y*=b.y, a.z*=b.z; return a; }
    friend Vector3 &operator *= (Vector3 &a, const float b)
    { a.x*=b, a.y*=b, a.z*=b; return a; }
    friend Vector3 &operator /= (Vector3 &a, const Vector3& b)
    { a.x/=b.x, a.y/=b.y, a.z/=b.z; return a; }
    friend Vector3 &operator /= (Vector3 &a, const float b)
    { a.x/=b, a.y/=b, a.z/=b; return a; }

    float length2 () const { return this->dot(*this); }
    float length () const { return ::sqrtf(length2()); }

    float distance(const Vector3& o) const
    { return (*this - o).length(); }
    float distance2(const Vector3& o) const
    { return (*this - o).length2(); }

    float dot(const Vector3& o) const { return x*o.x + y*o.y + z*o.z; }

    float normalise() { float l=length(); *this /= l; return l; }

    Vector3 cross (const Vector3& o) const
    { return Vector3(y*o.z - z*o.y,  z*o.x - x*o.z,  x*o.y - y*o.x); }

    Vector3 midPoint (const Vector3& o) const
    { return (*this+o)/2; }

    Radian angleBetween (const Vector3& o) const
    {
        float lp = ::sqrtf(length2() * o.length2());
        return Radian(acosf(this->dot(o) / lp));
    }

    bool isZeroLength (void) { return length2() < 1e-12; }

    // taken from OGRE (MIT license)
    Quaternion getRotationTo (Vector3 v1)
    {
        // Based on Stan Melax's article in Game Programming Gems
        // Copy, since cannot modify local
        Vector3 v0 = *this;
        v0.normalise();
        v1.normalise();

        float d = v0.dot(v1);
        // If dot == 1, vectors are the same
        if (d >= 1.0f) return Quaternion(1,0,0,0);

        if (d < (1e-6f - 1.0f)) {

            // rotate 180 degrees about the fallback axis
            // Generate an axis
            Vector3 axis = Vector3(1,0,0).cross(*this);
            if (axis.isZeroLength()) // pick another if colinear
                axis = Vector3(0,1,0).cross(*this);
            axis.normalise();
            return Quaternion(0, axis.x, axis.y, axis.z);

        } else {

            float s = ::sqrtf((1+d)*2);
            Vector3 axis = v0.cross(v1) / s;
            return Quaternion(s*0.5f, axis.x, axis.y, axis.z).normalisedCopy();

        }
    }

    // taken from OGRE (MIT license)
    Quaternion getRotationTo (Vector3 v1, const Vector3& fbaxis) const
    {
        // Based on Stan Melax's article in Game Programming Gems
        // Copy, since cannot modify local
        Vector3 v0 = *this;
        v0.normalise();
        v1.normalise();

        float d = v0.dot(v1);
        // If dot == 1, vectors are the same
        if (d >= 1.0f) return Quaternion(1,0,0,0);

        if (d < (1e-6f - 1.0f)) {

            // rotate 180 degrees about the fallback axis
            return Quaternion(0, fbaxis.x, fbaxis.y, fbaxis.z);

        } else {

            float s = ::sqrtf((1+d)*2);
            Vector3 axis = v0.cross(v1) / s;
            return Quaternion(s*0.5f, axis.x, axis.y, axis.z).normalisedCopy();

        }
    }

    Vector3 normalisedCopy (void) const
    { return *this / this->length(); }

    Vector3 reflect(const Vector3& normal) const
    { return *this - (2 * this->dot(normal) * normal); }

    /// Check whether this vector contains valid values
    bool isNaN (void) const
    { return my_isnan(x) || my_isnan(y) || my_isnan(z); }

    friend std::ostream& operator << (std::ostream& o, const Vector3& v)
    {
        o << "Vector3(" << v.x << ", " << v.y << ", " << v.z << ")";
        return o;
    }
};

// rotation of a vector by a quaternion
inline Vector3 operator * (const Quaternion &q, const Vector3& v)
{
    // nVidia SDK implementation
    Vector3 axis(q.x, q.y, q.z);
    Vector3 uv = axis.cross(v);
    Vector3 uuv = axis.cross(uv);
    uv *= (2.0f * q.w);
    uuv *= 2.0f;
    return v + uv + uuv;
}

inline Quaternion::Quaternion(const Radian& a, const Vector3& axis)
{
    float ha ( 0.5f*a.inRadians() );
    float s = sinf(ha);
    w = cosf(ha);
    x = s*axis.x;
    y = s*axis.y;
    z = s*axis.z;
}

// }}}


// {{{ Transform

struct Transform {

    Quaternion r; // rotation
    Vector3 p; // position

    Transform (void) { }
    Transform (const Quaternion &r_, const Vector3 &p_) : r(r_), p(p_) { }

    friend Vector3 operator * (const Transform &a, const Vector3 &b)
    { return a.p + a.r*b; }

    friend Transform operator * (const Transform &a, const Transform &b)
    { return Transform(a.r*b.r, a.p + a.r*b.p); }

};

// }}}

#endif
