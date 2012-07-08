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

#ifndef lua_userdata_dependency_tracker_funcs_h
#define lua_userdata_dependency_tracker_funcs_h

template <typename T>
static void vec_nullify_remove (std::vector<void**>& v, T **ud)
{
        void *const* ptr = (void *const*) ud; //thankyou c++
        std::vector<void**>::iterator i = std::find(v.begin(),v.end(),ptr);
        if (i==v.end()) {
                CERR<<"gc'ing: Spurious userdata!"<<std::endl;
                app_fatal();
        }
        **i = NULL;
        v.erase(i);
}

static void vec_nullify_remove_all (std::vector<void**>& v)
{
        for (std::vector<void**>::iterator i = v.begin(); i!=v.end() ; i++) {
                **i = NULL;
        }
        v.clear();
}

template <typename T, typename T2>
static void map_remove_only (std::map<T,T2>& m, T k)
{
        typename std::map<T,T2>::iterator i = m.find(k);
        if (i==m.end()) {
                CERR<<"gc'ing: Map does not contain item!"<<std::endl;
                app_fatal();
        }
        m.erase(i);
}

template <typename T>
static void map_nullify_remove (std::map<T,std::vector<void**> >&m, T k)
{
        vec_nullify_remove_all (m[k]);
        map_remove_only(m,k);
}

template <typename T>
static void map_nullify_remove_all (std::map<T*,std::vector<void**> >& map)
{
        typedef typename std::map<T*,std::vector<void**> >::iterator iter;
        for (iter i = map.begin(); i!=map.end() ; i++) {
                vec_nullify_remove_all(i->second);
        }
        map.clear();
}

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
