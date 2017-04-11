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

#ifndef NAVIGATIONMANAGER_H
#define NAVIGATIONMANAGER_H
#include "navigation_interfaces.h"
#include "DetourNavMesh.h"
#include "Recast.h"
#include "chunky_tri_mesh.h"

/// These are just sample areas to use consistent values across the samples.
/// The use should specify these base on his needs.
enum SamplePolyAreas
{
    SAMPLE_POLYAREA_GROUND,
    SAMPLE_POLYAREA_WATER,
    SAMPLE_POLYAREA_ROAD,
    SAMPLE_POLYAREA_DOOR,
    SAMPLE_POLYAREA_GRASS,
    SAMPLE_POLYAREA_JUMP,
};
enum SamplePolyFlags
{
    SAMPLE_POLYFLAGS_WALK = 0x01,        // Ability to walk (ground, grass, road)
    SAMPLE_POLYFLAGS_SWIM = 0x02,        // Ability to swim (water).
    SAMPLE_POLYFLAGS_DOOR = 0x04,        // Ability to move through doors.
    SAMPLE_POLYFLAGS_JUMP = 0x08,        // Ability to jump.
    SAMPLE_POLYFLAGS_DISABLED = 0x10,        // Disabled polygon
    SAMPLE_POLYFLAGS_ALL = 0xffff    // All abilities.
};

enum SamplePartitionType
{
    SAMPLE_PARTITION_WATERSHED,
    SAMPLE_PARTITION_MONOTONE,
    SAMPLE_PARTITION_LAYERS,
};

class NavigationManager
{
protected:
    class InputGeom* m_geom;
    class dtNavMesh* m_navMesh;
    class dtNavMeshQuery* m_navQuery;
    class dtCrowd* m_crowd;

    unsigned char m_navMeshDrawFlags;

    float m_cellSize;
    float m_cellHeight;
    float m_agentHeight;
    float m_agentRadius;
    float m_agentMaxClimb;
    float m_agentMaxSlope;
    float m_regionMinSize;
    float m_regionMergeSize;
    float m_edgeMaxLen;
    float m_edgeMaxError;
    float m_vertsPerPoly;
    float m_detailSampleDist;
    float m_detailSampleMaxError;
    int m_partitionType;

    BuildContext* m_ctx;

    bool m_keepInterResults;

    struct LinearAllocator* m_talloc;
    struct FastLZCompressor* m_tcomp;
    struct MeshProcess* m_tmproc;

    class dtTileCache* m_tileCache;

    float m_cacheBuildTimeMs;
    int m_cacheCompressedSize;
    int m_cacheRawSize;
    int m_cacheLayerCount;
    int m_cacheBuildMemUsage;
    
    enum DrawMode
    {
        DRAWMODE_NAVMESH,
        DRAWMODE_NAVMESH_TRANS,
        DRAWMODE_NAVMESH_BVTREE,
        DRAWMODE_NAVMESH_NODES,
        DRAWMODE_NAVMESH_PORTALS,
        DRAWMODE_NAVMESH_INVIS,
        DRAWMODE_MESH,
        DRAWMODE_CACHE_BOUNDS,
        MAX_DRAWMODE
    };
    
    DrawMode m_drawMode;
    
    int m_maxTiles;
    int m_maxPolysPerTile;
    float m_tileSize;
    
    // convex volumes:
    int m_areaType;
    float m_polyOffset;
    float m_boxHeight;
    float m_boxDescent;
    static const int MAX_PTS = 12;
    float m_pts[MAX_PTS * 3];
    int m_npts;
    int m_hull[MAX_PTS];
    int m_nhull;
public:
    NavigationManager();
    ~NavigationManager();

    //setters:
    // Rasterization:
    void setCellSize(float val) { m_cellSize = val; };
    void setCellHeight(float val) { m_cellHeight = val; };
    // Agent:
    void setAgentHeight(float val) { m_agentHeight = val; };
    void setAgentRadius(float val) { m_agentRadius = val; };
    void setAgentMaxClimb(float val) { m_agentMaxClimb = val; };
    void setAgentMaxSlope(float val) { m_agentMaxSlope = val; };
    // Region:
    void setRegionMinSize(float val) { m_regionMinSize = val; };
    void setRegionMergeSize(float val) { m_regionMergeSize = val; };
    // Partitioning:
    void setPartitionType(int val) { m_partitionType = val; };
    // Poligonization
    void setEdgeMaxLen(float val) { m_edgeMaxLen = val; };
    void setEdgeMaxError(float val) { m_edgeMaxError = val; };
    void setVertsPerPoly(float val) { m_vertsPerPoly = val; };
    // Detail Mesh
    void setDetailSampleDist(float val) { m_detailSampleDist = val; };
    void setDetailSampleMaxError(float val) { m_detailSampleMaxError = val; };
    void setKeepInterResults(bool val) { m_keepInterResults = val; };
    // Tiling:
    void setTileSize(float val) { m_tileSize = val; };

    // getters:
    float getCellSize(void) { return m_cellSize; };
    float getCellHeight(void) { return m_cellHeight; };
    // Agent:
    float getAgentMaxClimb(void) { return m_agentMaxClimb; };
    float getAgentMaxSlope(void) { return m_agentMaxSlope; };
    // Region:
    float getRegionMinSize(void) { return m_regionMinSize; };
    float getRegionMergeSize(void) { return m_regionMergeSize; };
    // Partitioning:
    int getPartitionType(void) { return m_partitionType; };
    // Poligonization
    float getEdgeMaxLen(void) { return m_edgeMaxLen; };
    float getEdgeMaxError(void) { return m_edgeMaxError; };
    float getVertsPerPoly(void) { return m_vertsPerPoly; };
    // Detail Mesh
    float getDetailSampleDist(void) { return m_detailSampleDist; };
    float getDetailSampleMaxError(void) { return m_detailSampleMaxError; };
    bool getKeepInterResults(void) { return m_keepInterResults; };
    // Tiling:
    float getTileSize(void) { return m_tileSize; };

    class CrowdTool* m_crowdTool;
    bool load(const char* path);
    void freeNavmesh();

    void removeConvexVolume(float* p);
    void createConvexVolume(float* p);

    void drawConvexVolume(DebugDrawGL* dd);

    dtTileCache* getTileCache(void) { return m_tileCache; };

    void updateMaxTiles();
    void render();
    void changeMesh(class InputGeom* geom);
    bool build();
    void update(const float dt);

    void getTilePos(const float* pos, int& tx, int& ty);
    
    void renderCachedTile(const int tx, const int ty, const int type);
    void renderCachedTileOverlay(const int tx, const int ty, double* proj, double* model, int* view);

    void addTempObstacle(const float* pos);
    void removeTempObstacle(const float* sp, const float* sq);
    void clearAllTempObstacles();

    bool saveAll(const char* path);
    bool loadAll(const char* path);

    void setContext(BuildContext* ctx) { m_ctx = ctx; }

    void step();

    class InputGeom* getInputGeom() { return m_geom; }
    class dtNavMesh* getNavMesh() { return m_navMesh; }
    class dtNavMeshQuery* getNavMeshQuery() { return m_navQuery; }
    class dtCrowd* getCrowd() { return m_crowd; }
    float getAgentRadius() { return m_agentRadius; }
    float getAgentHeight() { return m_agentHeight; }
    float getAgentClimb() { return m_agentMaxClimb; }
    const float* getBoundsMin();
    const float* getBoundsMax();

    inline unsigned char getNavMeshDrawFlags() const { return m_navMeshDrawFlags; }
    inline void setNavMeshDrawFlags(unsigned char flags) { m_navMeshDrawFlags = flags; }

    void resetCommonSettings();
};

#endif // NAVIGATIONMANAGER_H
