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

#ifndef lua_userdata_dependency_tracker_h
#define lua_userdata_dependency_tracker_h

struct scnmgr_maps {
        std::vector<void**> userData;
        std::map<Ogre::SceneNode*,std::vector<void**> > sceneNodes;
        std::map<Ogre::Camera*,std::vector<void**> > cameras;
        std::map<Ogre::Entity*,std::vector<void**> > entities;
        std::map<Ogre::ManualObject*,std::vector<void**> > manobjs;
        std::map<Ogre::Light*,std::vector<void**> > lights;
        std::map<Ogre::ParticleSystem*,std::vector<void**> > psyss;
        std::map<Ogre::StaticGeometry*,std::vector<void**> > statgeoms;
        std::map<Ogre::InstancedGeometry*,std::vector<void**> > instgeoms;
};

struct render_target_maps {
        std::vector<void**> userData;
        std::map<Ogre::Viewport*,std::vector<void**> > viewports;
};

struct UserDataTables {
        std::map<Ogre::SceneManager*,struct scnmgr_maps> scnmgrs;
        std::map<Ogre::RenderTarget*,struct render_target_maps> rts;
};

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
