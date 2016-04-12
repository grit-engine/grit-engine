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

#include "float.h"

#include "ray.h"


// Get the mesh information for the given mesh.  Code found on this forum link:
// http://www.ogre3d.org/wiki/index.php/RetrieveVertexData
void get_mesh_triangles(const Ogre::MeshPtr mesh,
                        size_t &vertex_count,
                        Ogre::Vector3* &vertices,
                        size_t &index_count,
                        unsigned long* &indices,
                        const Ogre::Vector3 &position,
                        const Ogre::Quaternion &orient,
                        const Ogre::Vector3 &scale)
{
        bool added_shared = false;

        vertex_count = index_count = 0;

        // Calculate how many vertices and indices we're going to need
        for (unsigned short i=0 ; i<mesh->getNumSubMeshes() ; ++i) {
                Ogre::SubMesh* submesh = mesh->getSubMesh (i);

                // We only need to add the shared vertices once
                if (!submesh->useSharedVertices) {
                        vertex_count += submesh->vertexData->vertexCount;
                } else if (!added_shared) {
                        vertex_count += mesh->sharedVertexData->vertexCount;
                        added_shared = true;
                }

                // Add the indices
                index_count += submesh->indexData->indexCount;
        }


        // Allocate space for the vertices and indices
        vertices = new Ogre::Vector3[vertex_count];
        indices = new unsigned long[index_count];

        added_shared = false;
        size_t current_offset = 0;
        size_t shared_offset = 0;
        size_t next_offset = 0;
        size_t index_offset = 0;

        // Run through the submeshes again, adding the data into the arrays
        for (unsigned short i=0; i<mesh->getNumSubMeshes(); i++) {
                Ogre::SubMesh* submesh = mesh->getSubMesh(i);

                Ogre::VertexData* vertex_data = submesh->useSharedVertices ?
                                   mesh->sharedVertexData : submesh->vertexData;


                if(!submesh->useSharedVertices ||
                   (submesh->useSharedVertices && !added_shared)) {
                        if(submesh->useSharedVertices) {
                                added_shared = true;
                                shared_offset = current_offset;
                        }

                        const Ogre::VertexElement* posElem = vertex_data->
                                vertexDeclaration->
                                findElementBySemantic(Ogre::VES_POSITION);

                        Ogre::HardwareVertexBufferSharedPtr vbuf =
                                vertex_data->vertexBufferBinding->
                                getBuffer(posElem->getSource());

                        unsigned char* vertex = static_cast<unsigned char*>
                              (vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

                        // There is _no_ baseVertexPointerToElement() which
                        // takes an Ogre::Real or a double as second argument.
                        // So make it float, to avoid trouble when Ogre::Real
                        // will be comiled/typedefed as double: Ogre::Real*
                        // pReal;

                        float* pReal;

                        for (size_t j=0 ; j<vertex_data->vertexCount ;
                             ++j, vertex+=vbuf->getVertexSize()) {
                                posElem->
                                     baseVertexPointerToElement(vertex, &pReal);

                                Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);

                                vertices[current_offset+j] =
                                        (orient*(pt*scale)) + position;
                        }

                        vbuf->unlock();
                        next_offset += vertex_data->vertexCount;
                }


                Ogre::IndexData* index_data = submesh->indexData;
                size_t numTris = index_data->indexCount / 3;
                Ogre::HardwareIndexBufferSharedPtr ibuf =
                        index_data->indexBuffer;

                bool use32bitindexes =
                        (ibuf->getType()==Ogre::HardwareIndexBuffer::IT_32BIT);

                unsigned long*  pLong = static_cast<unsigned long*>
                        (ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
                unsigned short* pShort = reinterpret_cast<unsigned short*>
                        (pLong);


                size_t offset = (submesh->useSharedVertices) ?
                        shared_offset : current_offset;

                if (use32bitindexes) {
                        for (size_t k=0 ; k<numTris*3 ; ++k) {
                                indices[index_offset++] = pLong[k] +
                                             static_cast<unsigned long>(offset);
                        }
                } else {
                        for (size_t k=0 ; k<numTris*3 ; ++k) {
                                indices[index_offset++] =
                                        static_cast<unsigned long>(pShort[k]) +
                                        static_cast<unsigned long>(offset);
                        }
                }

                ibuf->unlock();
                current_offset = next_offset;
        }
}


// raycast from a point in to the scene.
// returns success or failure.
// on success the point is returned in the result.
Ogre::Entity *poly_ray_cast(Ogre::RaySceneQuery *rsq,
                            const Ogre::Vector3& point,
                            const Ogre::Vector3& direction,
                            Ogre::Vector3& result)
{
        // create the ray to test
        Ogre::Ray ray(point,direction);

        rsq->setRay(ray);
        Ogre::RaySceneQueryResult query_result = rsq->execute();

        Ogre::Real closest_distance = FLT_MAX;
        Ogre::Vector3 closest_result(0,0,0);
        Ogre::Entity *closest_entity = NULL;
        for (size_t qr_idx=0 ; qr_idx < query_result.size() ; qr_idx++) {

                Ogre::RaySceneQueryResultEntry& result = query_result[qr_idx];

                if (closest_distance < result.distance) break;

                if (result.movable == NULL) continue;
                if (result.movable->getMovableType().compare("Entity")!=0)
                        continue;

                Ogre::Entity *pentity = static_cast<Ogre::Entity*>
                                                               (result.movable);

                std::cout<<"Ray hit: "<<pentity->getName()<<" mesh: "
                         <<pentity->getMesh()->getName()<<std::endl;



                size_t vertex_count;
                size_t index_count;
                Ogre::Vector3 *vertices;
                unsigned long *indices;

                Ogre::MeshPtr mesh = pentity->getMesh();
                Ogre::Node *node = pentity->getParentNode();

                get_mesh_triangles(mesh,
                                   vertex_count, vertices, index_count, indices,
                                   node->_getDerivedPosition(),
                                   node->_getDerivedOrientation(),
                                   node->getScale());

                bool found_something = false;
                for (size_t i=0 ; i<index_count ; i += 3) {

                        std::pair<bool, Ogre::Real> hit =
                                Ogre::Math::intersects(ray,
                                                       vertices[indices[i]],
                                                       vertices[indices[i+1]],
                                                       vertices[indices[i+2]]);

                        if (hit.first) { // if the ray hits this triangle
                                if (hit.second < closest_distance) {
                                        // this is the closest so far
                                        closest_distance = hit.second;
                                        found_something = true;
                                }
                        }
                }

                // if we found a new closest raycast for this object, update the
                // closest_result before moving on to the next object.
                if (found_something) {
                        closest_result = ray.getPoint(closest_distance);
                        closest_entity = pentity;
                }

                // free the verticies and indicies memory
                delete[] vertices;
                delete[] indices;
        }

        // return the result
        if (closest_entity!=NULL)
                result = closest_result;
        return closest_entity;
}


// vim: shiftwidth=8:tabstop=8:expandtab
