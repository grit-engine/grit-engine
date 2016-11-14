//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include "mesh_loader_obj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>

rcMeshLoaderObj::rcMeshLoaderObj() :
	m_scale(1.0f),
	m_verts(0),
	m_tris(0),
	m_normals(0),
	m_vertCount(0),
	m_triCount(0)
{
}

rcMeshLoaderObj::~rcMeshLoaderObj()
{
	delete [] m_verts;
	delete [] m_normals;
	delete [] m_tris;
}
		
void rcMeshLoaderObj::addVertex(float x, float y, float z, int& cap)
{
	if (m_vertCount+1 > cap)
	{
		cap = !cap ? 8 : cap*2;
		float* nv = new float[cap*3];
		if (m_vertCount)
			memcpy(nv, m_verts, m_vertCount*3*sizeof(float));
		delete [] m_verts;
		m_verts = nv;
	}
	float* dst = &m_verts[m_vertCount*3];

	*dst++ = x*m_scale;
	*dst++ = y*m_scale;
	*dst++ = z*m_scale;
	m_vertCount++;
}

void rcMeshLoaderObj::addTriangle(int a, int b, int c, int& cap)
{
	if (m_triCount+1 > cap)
	{
		cap = !cap ? 8 : cap*2;
		int* nv = new int[cap*3];
		if (m_triCount)
			memcpy(nv, m_tris, m_triCount*3*sizeof(int));
		delete [] m_tris;
		m_tris = nv;
	}
	int* dst = &m_tris[m_triCount*3];
	*dst++ = a;
	*dst++ = b;
	*dst++ = c;
	m_triCount++;
}

static char* parseRow(char* buf, char* bufEnd, char* row, int len)
{
	bool start = true;
	bool done = false;
	int n = 0;
	while (!done && buf < bufEnd)
	{
		char c = *buf;
		buf++;
		// multirow
		switch (c)
		{
			case '\\':
				break;
			case '\n':
				if (start) break;
				done = true;
				break;
			case '\r':
				break;
			case '\t':
			case ' ':
				if (start) break;
			default:
				start = false;
				row[n++] = c;
				if (n >= len-1)
					done = true;
				break;
		}
	}
	row[n] = '\0';
	return buf;
}

static int parseFace(char* row, int* data, int n, int vcnt)
{
	int j = 0;
	while (*row != '\0')
	{
		// Skip initial white space
		while (*row != '\0' && (*row == ' ' || *row == '\t'))
			row++;
		char* s = row;
		// Find vertex delimiter and terminated the string there for conversion.
		while (*row != '\0' && *row != ' ' && *row != '\t')
		{
			if (*row == '/') *row = '\0';
			row++;
		}
		if (*s == '\0')
			continue;
		int vi = atoi(s);
		data[j++] = vi < 0 ? vi+vcnt : vi-1;
		if (j >= n) return j;
	}
	return j;
}

bool rcMeshLoaderObj::load(const std::string& filename)
{
	char* buf = 0;
	FILE* fp = fopen(filename.c_str(), "rb");
	if (!fp)
		return false;
	if (fseek(fp, 0, SEEK_END) != 0)
	{
		fclose(fp);
		return false;
	}
	long bufSize = ftell(fp);
	if (bufSize < 0)
	{
		fclose(fp);
		return false;
	}
	if (fseek(fp, 0, SEEK_SET) != 0)
	{
		fclose(fp);
		return false;
	}
	buf = new char[bufSize];
	if (!buf)
	{
		fclose(fp);
		return false;
	}
	size_t readLen = fread(buf, bufSize, 1, fp);
	fclose(fp);

	if (readLen != 1)
	{
		delete[] buf;
		return false;
	}

	char* src = buf;
	char* srcEnd = buf + bufSize;
	char row[512];
	int face[32];
	float x,y,z;
	int nv;
	int vcap = 0;
	int tcap = 0;
	
	while (src < srcEnd)
	{
		// Parse one row
		row[0] = '\0';
		src = parseRow(src, srcEnd, row, sizeof(row)/sizeof(char));
		// Skip comments
		if (row[0] == '#') continue;
		if (row[0] == 'v' && row[1] != 'n' && row[1] != 't')
		{
			// Vertex pos
			sscanf(row+1, "%f %f %f", &x, &y, &z);
			addVertex(x, y, z, vcap);
		}
		if (row[0] == 'f')
		{
			// Faces
			nv = parseFace(row+1, face, 32, m_vertCount);
			for (int i = 2; i < nv; ++i)
			{
				const int a = face[0];
				const int b = face[i-1];
				const int c = face[i];
				if (a < 0 || a >= m_vertCount || b < 0 || b >= m_vertCount || c < 0 || c >= m_vertCount)
					continue;
				addTriangle(a, b, c, tcap);
			}
		}
	}

	delete [] buf;

	// Calculate normals.
	m_normals = new float[m_triCount*3];
	for (int i = 0; i < m_triCount*3; i += 3)
	{
		const float* v0 = &m_verts[m_tris[i]*3];
		const float* v1 = &m_verts[m_tris[i+1]*3];
		const float* v2 = &m_verts[m_tris[i+2]*3];
		float e0[3], e1[3];
		for (int j = 0; j < 3; ++j)
		{
			e0[j] = v1[j] - v0[j];
			e1[j] = v2[j] - v0[j];
		}
		float* n = &m_normals[i];
		n[0] = e0[1]*e1[2] - e0[2]*e1[1];
		n[1] = e0[2]*e1[0] - e0[0]*e1[2];
		n[2] = e0[0]*e1[1] - e0[1]*e1[0];
		float d = sqrtf(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
		if (d > 0)
		{
			d = 1.0f/d;
			n[0] *= d;
			n[1] *= d;
			n[2] *= d;
		}
	}
	
	m_filename = filename;
	return true;
}

// this method is a modified version of one taken from OgreCrowd http://ogre3d.org/forums/viewtopic.php?f=11&t=69781
bool rcMeshLoaderObj::convertGfxBody(std::vector<GfxBodyPtr> srcBodies)
{
    // TODO: This function should probably return early if srcBodies.size() == 0?
    APP_ASSERT(srcBodies.size() > 0);
	GfxBodyPtr ent = srcBodies[0];

//Convert all vertices and triangles to recast format
	const int numNodes = srcBodies.size();
	size_t *meshVertexCount = new size_t[numNodes];
	size_t *meshIndexCount = new size_t[numNodes];
	Ogre::Vector3 **meshVertices = new Ogre::Vector3*[numNodes];
	unsigned long **meshIndices = new unsigned long*[numNodes];

	m_vertCount = 0;
    int index_count = 0;
	size_t i = 0;
	for (std::vector<GfxBodyPtr>::iterator iter = srcBodies.begin(); iter != srcBodies.end(); iter++) {
		getMeshInformation((*iter)->mesh, meshVertexCount[i], meshVertices[i], meshIndexCount[i], meshIndices[i]);

		//total number of verts
		m_vertCount += meshVertexCount[i];
		//total number of indices
		index_count += meshIndexCount[i];

		i++;
	}

// DECLARE RECAST DATA BUFFERS USING THE INFO WE GRABBED ABOVE
	m_verts = new float[m_vertCount * 3];// *3 as verts holds x,y,&z for each verts in the array
	m_tris = new int[index_count];// tris in recast is really indices like ogre

	//convert index count into tri count
	m_triCount = index_count / 3; //although the tris array are indices the ntris is actual number of triangles, eg. indices/3;

	//copy all meshes verticies into single buffer and transform to world space relative to parentNode
	int vertsIndex = 0;
	int prevVerticiesCount = 0;
	int prevIndexCountTotal = 0;
	i = 0;
	for (std::vector<GfxBodyPtr>::iterator iter = srcBodies.begin(); iter != srcBodies.end(); iter++) {
		GfxBodyPtr ent = *iter;
		//find the transform between the reference node and this node
		Transform transform = ent->getWorldTransform();
		Vector3 vertexPos;
		for (size_t j = 0; j < meshVertexCount[i]; j++)
		{
			vertexPos = transform*from_ogre(meshVertices[i][j]);
			m_verts[vertsIndex + 0] = -vertexPos.x;
			m_verts[vertsIndex + 1] = vertexPos.z;
			m_verts[vertsIndex + 2] = vertexPos.y;
			vertsIndex += 3;
		}

		for (size_t j = 0; j < meshIndexCount[i]; j++)
		{
			m_tris[prevIndexCountTotal + j] = meshIndices[i][j] + prevVerticiesCount;
		}
		prevIndexCountTotal += meshIndexCount[i];
		prevVerticiesCount += meshVertexCount[i];

        APP_ASSERT(vertsIndex == prevVerticiesCount * 3);

		i++;
	}
    APP_ASSERT(prevVerticiesCount == m_vertCount);
    APP_ASSERT(prevIndexCountTotal == m_triCount * 3);

	//delete tempory arrays
	//TODO These probably could member variables, this would increase performance slightly
	delete[] meshVertices;
	delete[] meshIndices;
	delete[] meshVertexCount;
	delete[] meshIndexCount;

	m_normals = new float[m_triCount * 3];
	for (int i = 0; i < m_triCount * 3; i += 3)
	{
		const float* v0 = &m_verts[m_tris[i + 0] * 3];
		const float* v1 = &m_verts[m_tris[i + 1] * 3];
		const float* v2 = &m_verts[m_tris[i + 2] * 3];
		float e0[3], e1[3];
		for (int j = 0; j < 3; ++j)
		{
			e0[j] = (v1[j] - v0[j]);
			e1[j] = (v2[j] - v0[j]);
		}
		float* n = &m_normals[i];
		n[0] = ((e0[1] * e1[2]) - (e0[2] * e1[1]));
		n[1] = ((e0[2] * e1[0]) - (e0[0] * e1[2]));
		n[2] = ((e0[0] * e1[1]) - (e0[1] * e1[0]));

		float d = sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
		if (d > 0)
		{
			d = 1.0f / d;
			n[0] *= d;
			n[1] *= d;
			n[2] *= d;
		}
	}
	return true;
}

// Get the mesh information for the given mesh.  Code found on this Wiki link:
// http://www.ogre3d.org/tikiwiki/RetrieveVertexData
void getMeshInformation(const Ogre::MeshPtr mesh, size_t &vertex_count, Ogre::Vector3* &vertices,
	size_t &index_count, unsigned long* &indices, const Ogre::Vector3 &position,
	const Ogre::Quaternion &orient, const Ogre::Vector3 &scale)
{
	bool added_shared = false;
	size_t current_offset = 0;
	size_t shared_offset = 0;
	size_t next_offset = 0;
	size_t index_offset = 0;

	vertex_count = index_count = 0;

	// Calculate how many vertices and indices we're going to need
	for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
	{
		Ogre::SubMesh* submesh = mesh->getSubMesh(i);
		// We only need to add the shared vertices once
		if (submesh->useSharedVertices)
		{
			if (!added_shared)
			{
				vertex_count += mesh->sharedVertexData->vertexCount;
				added_shared = true;
			}
		}
		else
		{
			vertex_count += submesh->vertexData->vertexCount;
		}
		// Add the indices
		index_count += submesh->indexData->indexCount;
	}

	// Allocate space for the vertices and indices
	vertices = new Ogre::Vector3[vertex_count];
	indices = new unsigned long[index_count];

	added_shared = false;

	// Run through the submeshes again, adding the data into the arrays
	for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
	{
		Ogre::SubMesh* submesh = mesh->getSubMesh(i);

		Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

		if ((!submesh->useSharedVertices) || (submesh->useSharedVertices && !added_shared))
		{
			if (submesh->useSharedVertices)
			{
				added_shared = true;
				shared_offset = current_offset;
			}

			const Ogre::VertexElement* posElem =
				vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

			Ogre::HardwareVertexBufferSharedPtr vbuf =
				vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

			unsigned char* vertex =
				static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

			// There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
			//  as second argument. So make it float, to avoid trouble when Ogre::Real will
			//  be comiled/typedefed as double:
			//Ogre::Real* pReal;
			float* pReal;

			for (size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
			{
				posElem->baseVertexPointerToElement(vertex, &pReal);
				Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);
				vertices[current_offset + j] = (orient * (pt * scale)) + position;
			}

			vbuf->unlock();
			next_offset += vertex_data->vertexCount;
		}


		Ogre::IndexData* index_data = submesh->indexData;
		size_t numTris = index_data->indexCount / 3;
		Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

		bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

		size_t offset = (submesh->useSharedVertices) ? shared_offset : current_offset;

		if (use32bitindexes)
		{
            auto *pLong = static_cast<uint32_t*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
			for (size_t k = 0; k < numTris * 3; ++k)
			{
				indices[index_offset++] = pLong[k] + offset;
			}
		}
		else
		{
            auto *pShort = static_cast<uint16_t*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
			for (size_t k = 0; k < numTris * 3; ++k)
			{
				indices[index_offset++] = static_cast<unsigned long>(pShort[k]) + offset;
			}
		}

		ibuf->unlock();
		current_offset = next_offset;
	}
}

// TODO
bool rcMeshLoaderObj::convertRigidBody(std::vector<RigidBody*> srcBodies)
{
    (void) srcBodies;
	return true;
}
