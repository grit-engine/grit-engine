#include <Ogre.h>
#include <OgreSceneQuery.h>


// Get the mesh information for the given mesh.  Code found on this forum link:
// http://www.ogre3d.org/wiki/index.php/RetrieveVertexData
void get_mesh_triangles(const Ogre::MeshPtr mesh,
                        size_t &vertex_count,
                        Ogre::Vector3* &vertices,
                        size_t &index_count,
                        unsigned long* &indices,
                        const Ogre::Vector3 &position,
                        const Ogre::Quaternion &orient,
                        const Ogre::Vector3 &scale);


// raycast from a point in to the scene.
// returns success or failure.
// on success the point is returned in the result.
Ogre::Entity *poly_ray_cast(Ogre::RaySceneQuery *rsq,
                            const Ogre::Vector3& point,
                            const Ogre::Vector3& direction,
                            Ogre::Vector3& result);

// vim: shiftwidth=8:tabstop=8:expandtab
