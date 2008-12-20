#ifndef lua_userdata_dependency_tracker_h
#define lua_userdata_dependency_tracker_h

struct scnmgr_maps {
        std::vector<void**> userData;
        std::map<Ogre::SceneNode*,std::vector<void**> > sceneNodes;
        std::map<Ogre::Camera*,std::vector<void**> > cameras;
        std::map<Ogre::Entity*,std::vector<void**> > entities;
        std::map<Ogre::ManualObject*,std::vector<void**> > manobjs;
        std::map<Ogre::Light*,std::vector<void**> > lights;
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
