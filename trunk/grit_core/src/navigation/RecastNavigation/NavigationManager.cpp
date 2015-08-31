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

// This is a modified version of Sample_TempObstacles.cpp from Recast Demo

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <new>
#include "InputGeom.h"
#include "NavigationManager.h"
#include "Recast.h"
#include "RecastDebugDraw.h"
#include "DetourAssert.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourDebugDraw.h"
#include "DetourCommon.h"
#include "DetourTileCache.h"
#include "CrowdManager.h"
#include "RecastAlloc.h"
#include "RecastAssert.h"
#include "fastlz.h"

#include "DetourNavMeshQuery.h"
#include "DetourCrowd.h"

#ifdef WIN32
#	define snprintf _snprintf
#endif

#include"navigation_system.h"

// This value specifies how many layers (or "floors") each navmesh tile is expected to have.
static const int EXPECTED_LAYERS_PER_TILE = 4;


static bool isectSegAABB(const float* sp, const float* sq,
						 const float* amin, const float* amax,
						 float& tmin, float& tmax)
{
	static const float EPS = 1e-6f;
	
	float d[3];
	rcVsub(d, sq, sp);
	tmin = 0;  // set to -FLT_MAX to get first hit on line
	tmax = FLT_MAX;		// set to max distance ray can travel (for segment)
	
	// For all three slabs
	for (int i = 0; i < 3; i++)
	{
		if (fabsf(d[i]) < EPS)
		{
			// Ray is parallel to slab. No hit if origin not within slab
			if (sp[i] < amin[i] || sp[i] > amax[i])
				return false;
		}
		else
		{
			// Compute intersection t value of ray with near and far plane of slab
			const float ood = 1.0f / d[i];
			float t1 = (amin[i] - sp[i]) * ood;
			float t2 = (amax[i] - sp[i]) * ood;
			// Make t1 be intersection with near plane, t2 with far plane
			if (t1 > t2) rcSwap(t1, t2);
			// Compute the intersection of slab intersections intervals
			if (t1 > tmin) tmin = t1;
			if (t2 < tmax) tmax = t2;
			// Exit with no collision as soon as slab intersection becomes empty
			if (tmin > tmax) return false;
		}
	}
	
	return true;
}

static int calcLayerBufferSize(const int gridWidth, const int gridHeight)
{
	const int headerSize = dtAlign4(sizeof(dtTileCacheLayerHeader));
	const int gridSize = gridWidth * gridHeight;
	return headerSize + gridSize*4;
}

struct FastLZCompressor : public dtTileCacheCompressor
{
	virtual int maxCompressedSize(const int bufferSize)
	{
		return (int)(bufferSize* 1.05f);
	}
	
	virtual dtStatus compress(const unsigned char* buffer, const int bufferSize,
							  unsigned char* compressed, const int /*maxCompressedSize*/, int* compressedSize)
	{
		*compressedSize = fastlz_compress((const void *const)buffer, bufferSize, compressed);
		return DT_SUCCESS;
	}
	
	virtual dtStatus decompress(const unsigned char* compressed, const int compressedSize,
								unsigned char* buffer, const int maxBufferSize, int* bufferSize)
	{
		*bufferSize = fastlz_decompress(compressed, compressedSize, buffer, maxBufferSize);
		return *bufferSize < 0 ? DT_FAILURE : DT_SUCCESS;
	}
};

struct LinearAllocator : public dtTileCacheAlloc
{
	unsigned char* buffer;
	int capacity;
	int top;
	int high;
	
	LinearAllocator(const int cap) : buffer(0), capacity(0), top(0), high(0)
	{
		resize(cap);
	}
	
	~LinearAllocator()
	{
		dtFree(buffer);
	}

	void resize(const int cap)
	{
		if (buffer) dtFree(buffer);
		buffer = (unsigned char*)dtAlloc(cap, DT_ALLOC_PERM);
		capacity = cap;
	}
	
	virtual void reset()
	{
		high = dtMax(high, top);
		top = 0;
	}
	
	virtual void* alloc(const int size)
	{
		if (!buffer)
			return 0;
		if (top+size > capacity)
			return 0;
		unsigned char* mem = &buffer[top];
		top += size;
		return mem;
	}
	
	virtual void free(void* /*ptr*/)
	{
		// Empty
	}
};

struct MeshProcess : public dtTileCacheMeshProcess
{
	InputGeom* m_geom;

	inline MeshProcess() : m_geom(0)
	{
	}

	inline void init(InputGeom* geom)
	{
		m_geom = geom;
	}
	
	virtual void process(struct dtNavMeshCreateParams* params,
						 unsigned char* polyAreas, unsigned short* polyFlags)
	{
		// Update poly flags from areas.
		for (int i = 0; i < params->polyCount; ++i)
		{
			if (polyAreas[i] == DT_TILECACHE_WALKABLE_AREA)
				polyAreas[i] = SAMPLE_POLYAREA_GROUND;

			if (polyAreas[i] == SAMPLE_POLYAREA_GROUND ||
				polyAreas[i] == SAMPLE_POLYAREA_GRASS ||
				polyAreas[i] == SAMPLE_POLYAREA_ROAD)
			{
				polyFlags[i] = SAMPLE_POLYFLAGS_WALK;
			}
			else if (polyAreas[i] == SAMPLE_POLYAREA_WATER)
			{
				polyFlags[i] = SAMPLE_POLYFLAGS_SWIM;
			}
			else if (polyAreas[i] == SAMPLE_POLYAREA_DOOR)
			{
				polyFlags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
			}
		}

		// Pass in off-mesh connections.
		if (m_geom)
		{
			params->offMeshConVerts = m_geom->getOffMeshConnectionVerts();
			params->offMeshConRad = m_geom->getOffMeshConnectionRads();
			params->offMeshConDir = m_geom->getOffMeshConnectionDirs();
			params->offMeshConAreas = m_geom->getOffMeshConnectionAreas();
			params->offMeshConFlags = m_geom->getOffMeshConnectionFlags();
			params->offMeshConUserID = m_geom->getOffMeshConnectionId();
			params->offMeshConCount = m_geom->getOffMeshConnectionCount();	
		}
	}
};

static const int MAX_LAYERS = 32;

struct TileCacheData
{
	unsigned char* data;
	int dataSize;
};

struct RasterizationContext
{
	RasterizationContext() :
		solid(0),
		triareas(0),
		lset(0),
		chf(0),
		ntiles(0)
	{
		memset(tiles, 0, sizeof(TileCacheData)*MAX_LAYERS);
	}
	
	~RasterizationContext()
	{
		rcFreeHeightField(solid);
		delete [] triareas;
		rcFreeHeightfieldLayerSet(lset);
		rcFreeCompactHeightfield(chf);
		for (int i = 0; i < MAX_LAYERS; ++i)
		{
			dtFree(tiles[i].data);
			tiles[i].data = 0;
		}
	}
	
	rcHeightfield* solid;
	unsigned char* triareas;
	rcHeightfieldLayerSet* lset;
	rcCompactHeightfield* chf;
	TileCacheData tiles[MAX_LAYERS];
	int ntiles;
};

static int rasterizeTileLayers(BuildContext* ctx, InputGeom* geom,
							   const int tx, const int ty,
							   const rcConfig& cfg,
							   TileCacheData* tiles,
							   const int maxTiles)
{
	if (!geom || !geom->getMesh() || !geom->getChunkyMesh())
	{
		ctx->log(RC_LOG_ERROR, "buildTile: Input mesh is not specified.");
		return 0;
	}
	
	FastLZCompressor comp;
	RasterizationContext rc;
	
	const float* verts = geom->getMesh()->getVerts();
	const int nverts = geom->getMesh()->getVertCount();
	const rcChunkyTriMesh* chunkyMesh = geom->getChunkyMesh();
	
	// Tile bounds.
	const float tcs = cfg.tileSize * cfg.cs;
	
	rcConfig tcfg;
	memcpy(&tcfg, &cfg, sizeof(tcfg));

	tcfg.bmin[0] = cfg.bmin[0] + tx*tcs;
	tcfg.bmin[1] = cfg.bmin[1];
	tcfg.bmin[2] = cfg.bmin[2] + ty*tcs;
	tcfg.bmax[0] = cfg.bmin[0] + (tx+1)*tcs;
	tcfg.bmax[1] = cfg.bmax[1];
	tcfg.bmax[2] = cfg.bmin[2] + (ty+1)*tcs;
	tcfg.bmin[0] -= tcfg.borderSize*tcfg.cs;
	tcfg.bmin[2] -= tcfg.borderSize*tcfg.cs;
	tcfg.bmax[0] += tcfg.borderSize*tcfg.cs;
	tcfg.bmax[2] += tcfg.borderSize*tcfg.cs;
	
	// Allocate voxel heightfield where we rasterize our input data to.
	rc.solid = rcAllocHeightfield();
	if (!rc.solid)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
		return 0;
	}
	if (!rcCreateHeightfield(ctx, *rc.solid, tcfg.width, tcfg.height, tcfg.bmin, tcfg.bmax, tcfg.cs, tcfg.ch))
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
		return 0;
	}
	
	// Allocate array that can hold triangle flags.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	rc.triareas = new unsigned char[chunkyMesh->maxTrisPerChunk];
	if (!rc.triareas)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'm_triareas' (%d).", chunkyMesh->maxTrisPerChunk);
		return 0;
	}
	
	float tbmin[2], tbmax[2];
	tbmin[0] = tcfg.bmin[0];
	tbmin[1] = tcfg.bmin[2];
	tbmax[0] = tcfg.bmax[0];
	tbmax[1] = tcfg.bmax[2];
	int cid[512];// TODO: Make grow when returning too many items.
	const int ncid = rcGetChunksOverlappingRect(chunkyMesh, tbmin, tbmax, cid, 512);
	if (!ncid)
	{
		return 0; // empty
	}
	
	for (int i = 0; i < ncid; ++i)
	{
		const rcChunkyTriMeshNode& node = chunkyMesh->nodes[cid[i]];
		const int* tris = &chunkyMesh->tris[node.i*3];
		const int ntris = node.n;
		
		memset(rc.triareas, 0, ntris*sizeof(unsigned char));
		rcMarkWalkableTriangles(ctx, tcfg.walkableSlopeAngle,
								verts, nverts, tris, ntris, rc.triareas);
		
		rcRasterizeTriangles(ctx, verts, nverts, tris, rc.triareas, ntris, *rc.solid, tcfg.walkableClimb);
	}
	
	// Once all geometry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	rcFilterLowHangingWalkableObstacles(ctx, tcfg.walkableClimb, *rc.solid);
	rcFilterLedgeSpans(ctx, tcfg.walkableHeight, tcfg.walkableClimb, *rc.solid);
	rcFilterWalkableLowHeightSpans(ctx, tcfg.walkableHeight, *rc.solid);
	
	
	rc.chf = rcAllocCompactHeightfield();
	if (!rc.chf)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
		return 0;
	}
	if (!rcBuildCompactHeightfield(ctx, tcfg.walkableHeight, tcfg.walkableClimb, *rc.solid, *rc.chf))
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
		return 0;
	}
	
	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(ctx, tcfg.walkableRadius, *rc.chf))
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
		return 0;
	}
	
	// (Optional) Mark areas.
	const ConvexVolume* vols = geom->getConvexVolumes();
	for (int i  = 0; i < geom->getConvexVolumeCount(); ++i)
	{
		rcMarkConvexPolyArea(ctx, vols[i].verts, vols[i].nverts,
							 vols[i].hmin, vols[i].hmax,
							 (unsigned char)vols[i].area, *rc.chf);
	}
	
	rc.lset = rcAllocHeightfieldLayerSet();
	if (!rc.lset)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'lset'.");
		return 0;
	}
	if (!rcBuildHeightfieldLayers(ctx, *rc.chf, tcfg.borderSize, tcfg.walkableHeight, *rc.lset))
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build heighfield layers.");
		return 0;
	}
	
	rc.ntiles = 0;
	for (int i = 0; i < rcMin(rc.lset->nlayers, MAX_LAYERS); ++i)
	{
		TileCacheData* tile = &rc.tiles[rc.ntiles++];
		const rcHeightfieldLayer* layer = &rc.lset->layers[i];
		
		// Store header
		dtTileCacheLayerHeader header;
		header.magic = DT_TILECACHE_MAGIC;
		header.version = DT_TILECACHE_VERSION;
		
		// Tile layer location in the navmesh.
		header.tx = tx;
		header.ty = ty;
		header.tlayer = i;
		dtVcopy(header.bmin, layer->bmin);
		dtVcopy(header.bmax, layer->bmax);
		
		// Tile info.
		header.width = (unsigned char)layer->width;
		header.height = (unsigned char)layer->height;
		header.minx = (unsigned char)layer->minx;
		header.maxx = (unsigned char)layer->maxx;
		header.miny = (unsigned char)layer->miny;
		header.maxy = (unsigned char)layer->maxy;
		header.hmin = (unsigned short)layer->hmin;
		header.hmax = (unsigned short)layer->hmax;

		dtStatus status = dtBuildTileCacheLayer(&comp, &header, layer->heights, layer->areas, layer->cons,
												&tile->data, &tile->dataSize);
		if (dtStatusFailed(status))
		{
			return 0;
		}
	}

	// Transfer ownsership of tile data from build context to the caller.
	int n = 0;
	for (int i = 0; i < rcMin(rc.ntiles, maxTiles); ++i)
	{
		tiles[n++] = rc.tiles[i];
		rc.tiles[i].data = 0;
		rc.tiles[i].dataSize = 0;
	}
	
	return n;
}

void drawTiles(duDebugDraw* dd, dtTileCache* tc)
{
	unsigned int fcol[6];
	float bmin[3], bmax[3];

	for (int i = 0; i < tc->getTileCount(); ++i)
	{
		const dtCompressedTile* tile = tc->getTile(i);
		if (!tile->header) continue;
		
		tc->calcTightTileBounds(tile->header, bmin, bmax);
		
		const unsigned int col = duIntToCol(i,64);
		duCalcBoxColors(fcol, col, col);
		duDebugDrawBox(dd, bmin[0],bmin[1],bmin[2], bmax[0],bmax[1],bmax[2], fcol);
	}
	
	for (int i = 0; i < tc->getTileCount(); ++i)
	{
		const dtCompressedTile* tile = tc->getTile(i);
		if (!tile->header) continue;
		
		tc->calcTightTileBounds(tile->header, bmin, bmax);
		
		const unsigned int col = duIntToCol(i,255);
		const float pad = tc->getParams()->cs * 0.1f;
		duDebugDrawBoxWire(dd, bmin[0]-pad,bmin[1]-pad,bmin[2]-pad,
						   bmax[0]+pad,bmax[1]+pad,bmax[2]+pad, col, 2.0f);
	}

}

enum DrawDetailType
{
	DRAWDETAIL_AREAS,
	DRAWDETAIL_REGIONS,
	DRAWDETAIL_CONTOURS,
	DRAWDETAIL_MESH,
};

void drawDetail(duDebugDraw* dd, dtTileCache* tc, const int tx, const int ty, int type)
{
	struct TileCacheBuildContext
	{
		inline TileCacheBuildContext(struct dtTileCacheAlloc* a) : layer(0), lcset(0), lmesh(0), alloc(a) {}
		inline ~TileCacheBuildContext() { purge(); }
		void purge()
		{
			dtFreeTileCacheLayer(alloc, layer);
			layer = 0;
			dtFreeTileCacheContourSet(alloc, lcset);
			lcset = 0;
			dtFreeTileCachePolyMesh(alloc, lmesh);
			lmesh = 0;
		}
		struct dtTileCacheLayer* layer;
		struct dtTileCacheContourSet* lcset;
		struct dtTileCachePolyMesh* lmesh;
		struct dtTileCacheAlloc* alloc;
	};

	dtCompressedTileRef tiles[MAX_LAYERS];
	const int ntiles = tc->getTilesAt(tx,ty,tiles,MAX_LAYERS);

	dtTileCacheAlloc* talloc = tc->getAlloc();
	dtTileCacheCompressor* tcomp = tc->getCompressor();
	const dtTileCacheParams* params = tc->getParams();

	for (int i = 0; i < ntiles; ++i)
	{
		const dtCompressedTile* tile = tc->getTileByRef(tiles[i]);

		talloc->reset();

		TileCacheBuildContext bc(talloc);
		const int walkableClimbVx = (int)(params->walkableClimb / params->ch);
		dtStatus status;
		
		// Decompress tile layer data. 
		status = dtDecompressTileCacheLayer(talloc, tcomp, tile->data, tile->dataSize, &bc.layer);
		if (dtStatusFailed(status))
			return;
		if (type == DRAWDETAIL_AREAS)
		{
			duDebugDrawTileCacheLayerAreas(dd, *bc.layer, params->cs, params->ch);
			continue;
		}

		// Build navmesh
		status = dtBuildTileCacheRegions(talloc, *bc.layer, walkableClimbVx);
		if (dtStatusFailed(status))
			return;
		if (type == DRAWDETAIL_REGIONS)
		{
			duDebugDrawTileCacheLayerRegions(dd, *bc.layer, params->cs, params->ch);
			continue;
		}
		
		bc.lcset = dtAllocTileCacheContourSet(talloc);
		if (!bc.lcset)
			return;
		status = dtBuildTileCacheContours(talloc, *bc.layer, walkableClimbVx,
										  params->maxSimplificationError, *bc.lcset);
		if (dtStatusFailed(status))
			return;
		if (type == DRAWDETAIL_CONTOURS)
		{
			duDebugDrawTileCacheContours(dd, *bc.lcset, tile->header->bmin, params->cs, params->ch);
			continue;
		}
		
		bc.lmesh = dtAllocTileCachePolyMesh(talloc);
		if (!bc.lmesh)
			return;
		status = dtBuildTileCachePolyMesh(talloc, *bc.lcset, *bc.lmesh);
		if (dtStatusFailed(status))
			return;

		if (type == DRAWDETAIL_MESH)
		{
			duDebugDrawTileCachePolyMesh(dd, *bc.lmesh, tile->header->bmin, params->cs, params->ch);
			continue;
		}

	}
}

void drawDetailOverlay(const dtTileCache* tc, const int tx, const int ty, double* proj, double* model, int* view)
{
	dtCompressedTileRef tiles[MAX_LAYERS];
	const int ntiles = tc->getTilesAt(tx,ty,tiles,MAX_LAYERS);
	if (!ntiles)
		return;
	
	const int rawSize = calcLayerBufferSize(tc->getParams()->width, tc->getParams()->height);
	
	char text[128];

	for (int i = 0; i < ntiles; ++i)
	{
		const dtCompressedTile* tile = tc->getTileByRef(tiles[i]);
		
		float pos[3];
		pos[0] = (tile->header->bmin[0]+tile->header->bmax[0])/2.0f;
		pos[1] = tile->header->bmin[1];
		pos[2] = (tile->header->bmin[2]+tile->header->bmax[2])/2.0f;
		//-- TODO: replace this using Ogre "MoveableTextOverlay"
		//GLdouble x, y, z;
		//if (gluProject((GLdouble)pos[0], (GLdouble)pos[1], (GLdouble)pos[2],
		//			   model, proj, view, &x, &y, &z))
		//{
		//	snprintf(text,128,"(%d,%d)/%d", tile->header->tx,tile->header->ty,tile->header->tlayer);
		//	imguiDrawText((int)x, (int)y-25, IMGUI_ALIGN_CENTER, text, imguiRGBA(0,0,0,220));
		//	snprintf(text,128,"Compressed: %.1f kB", tile->dataSize/1024.0f);
		//	imguiDrawText((int)x, (int)y-45, IMGUI_ALIGN_CENTER, text, imguiRGBA(0,0,0,128));
		//	snprintf(text,128,"Raw:%.1fkB", rawSize/1024.0f);
		//	imguiDrawText((int)x, (int)y-65, IMGUI_ALIGN_CENTER, text, imguiRGBA(0,0,0,128));
		//}
	}
}
		
dtObstacleRef hitTestObstacle(const dtTileCache* tc, const float* sp, const float* sq)
{
	float tmin = FLT_MAX;
	const dtTileCacheObstacle* obmin = 0;
	for (int i = 0; i < tc->getObstacleCount(); ++i)
	{
		const dtTileCacheObstacle* ob = tc->getObstacle(i);
		if (ob->state == DT_OBSTACLE_EMPTY)
			continue;
		
		float bmin[3], bmax[3], t0,t1;
		tc->getObstacleBounds(ob, bmin,bmax);
		
		if (isectSegAABB(sp,sq, bmin,bmax, t0,t1))
		{
			if (t0 < tmin)
			{
				tmin = t0;
				obmin = ob;
			}
		}
	}
	return tc->getObstacleRef(obmin);
}
	
void drawObstacles(duDebugDraw* dd, const dtTileCache* tc)
{
	// Draw obstacles
	for (int i = 0; i < tc->getObstacleCount(); ++i)
	{
		const dtTileCacheObstacle* ob = tc->getObstacle(i);
		if (ob->state == DT_OBSTACLE_EMPTY) continue;
		float bmin[3], bmax[3];
		tc->getObstacleBounds(ob, bmin,bmax);

		unsigned int col = 0;
		if (ob->state == DT_OBSTACLE_PROCESSING)
			col = duRGBA(255,255,0,128);
		else if (ob->state == DT_OBSTACLE_PROCESSED)
			col = duRGBA(255,192,0,192);
		else if (ob->state == DT_OBSTACLE_REMOVING)
			col = duRGBA(220,0,0,128);

		duDebugDrawCylinder(dd, bmin[0],bmin[1],bmin[2], bmax[0],bmax[1],bmax[2], col);
		duDebugDrawCylinderWire(dd, bmin[0],bmin[1],bmin[2], bmax[0],bmax[1],bmax[2], duDarkenCol(col), 2);
	}
}

NavigationManager::NavigationManager() :
	m_keepInterResults(false),
	m_tileCache(0),
	m_cacheBuildTimeMs(0),
	m_cacheCompressedSize(0),
	m_cacheRawSize(0),
	m_cacheLayerCount(0),
	m_cacheBuildMemUsage(0),
	m_drawMode(DRAWMODE_NAVMESH),
	m_maxTiles(0),
	m_maxPolysPerTile(0),
	m_tileSize(48),

	m_geom(0),
	m_navMesh(0),
	m_navQuery(0),
	m_crowd(0),
	m_navMeshDrawFlags(DU_DRAWNAVMESH_OFFMESHCONS | DU_DRAWNAVMESH_CLOSEDLIST),
	m_ctx(0)
{
	m_navQuery = dtAllocNavMeshQuery();
	m_crowd = dtAllocCrowd();

	resetCommonSettings();
	
	m_talloc = new LinearAllocator(32000);
	m_tcomp = new FastLZCompressor;
	m_tmproc = new MeshProcess;

	m_crowdTool = new CrowdTool();
}

NavigationManager::~NavigationManager()
{
	dtFreeNavMeshQuery(m_navQuery);
	dtFreeNavMesh(m_navMesh);
	dtFreeCrowd(m_crowd);

	m_navMesh = 0;
	dtFreeTileCache(m_tileCache);
}

void NavigationManager::updateMaxTiles()
{
	if (m_geom)
	{
		const float* bmin = m_geom->getMeshBoundsMin();
		const float* bmax = m_geom->getMeshBoundsMax();
		char text[64];
		int gw = 0, gh = 0;
		rcCalcGridSize(bmin, bmax, m_cellSize, &gw, &gh);
		const int ts = (int)m_tileSize;
		const int tw = (gw + ts-1) / ts;
		const int th = (gh + ts-1) / ts;

		// Max tiles and max polys affect how the tile IDs are caculated.
		// There are 22 bits available for identifying a tile and a polygon.
		int tileBits = rcMin((int)dtIlog2(dtNextPow2(tw*th*EXPECTED_LAYERS_PER_TILE)), 14);
		if (tileBits > 14) tileBits = 14;
		int polyBits = 22 - tileBits;
		m_maxTiles = 1 << tileBits;
		m_maxPolysPerTile = 1 << polyBits;
	}
	else
	{
		m_maxTiles = 0;
		m_maxPolysPerTile = 0;
	}
}

// this is a modified function of Recast Debug Draw
// removed tile bounds and points
static void drawMeshTilex(duDebugDraw* dd, const dtNavMesh& mesh, const dtNavMeshQuery* query,
	const dtMeshTile* tile, unsigned char flags)
{
	dtPolyRef base = mesh.getPolyRefBase(tile);

	int tileNum = mesh.decodePolyIdTile(base);

	dd->depthMask(false);

	dd->begin(DU_DRAW_TRIS);
	for (int i = 0; i < tile->header->polyCount; ++i)
	{
		const dtPoly* p = &tile->polys[i];
		if (p->getType() == DT_POLYTYPE_OFFMESH_CONNECTION)	// Skip off-mesh links.
			continue;

		const dtPolyDetail* pd = &tile->detailMeshes[i];

		unsigned int col;
		if (query && query->isInClosedList(base | (dtPolyRef)i))
			col = duRGBA(255, 196, 0, 64);
		else
		{
			if (flags & DU_DRAWNAVMESH_COLOR_TILES)
			{
				col = duIntToCol(tileNum, 128);
			}
			else
			{
				if (p->getArea() == 0) // Treat zero area type as default.
					col = duRGBA(0, 192, 255, 64);
				else
					col = duIntToCol(p->getArea(), 64);
			}
		}

		for (int j = 0; j < pd->triCount; ++j)
		{
			const unsigned char* t = &tile->detailTris[(pd->triBase + j) * 4];
			for (int k = 0; k < 3; ++k)
			{
				if (t[k] < p->vertCount)
					dd->vertex(&tile->verts[p->verts[t[k]] * 3], col);
				else
					dd->vertex(&tile->detailVerts[(pd->vertBase + t[k] - p->vertCount) * 3], col);
			}
		}
	}
	dd->end();

	if (flags & DU_DRAWNAVMESH_OFFMESHCONS)
	{
		dd->begin(DU_DRAW_LINES, 2.0f);
		for (int i = 0; i < tile->header->polyCount; ++i)
		{
			const dtPoly* p = &tile->polys[i];
			if (p->getType() != DT_POLYTYPE_OFFMESH_CONNECTION)	// Skip regular polys.
				continue;

			unsigned int col, col2;
			if (query && query->isInClosedList(base | (dtPolyRef)i))
				col = duRGBA(255, 196, 0, 220);
			else
				col = duDarkenCol(duIntToCol(p->getArea(), 220));

			const dtOffMeshConnection* con = &tile->offMeshCons[i - tile->header->offMeshBase];
			const float* va = &tile->verts[p->verts[0] * 3];
			const float* vb = &tile->verts[p->verts[1] * 3];

			// Check to see if start and end end-points have links.
			bool startSet = false;
			bool endSet = false;
			for (unsigned int k = p->firstLink; k != DT_NULL_LINK; k = tile->links[k].next)
			{
				if (tile->links[k].edge == 0)
					startSet = true;
				if (tile->links[k].edge == 1)
					endSet = true;
			}

			// End points and their on-mesh locations.
			dd->vertex(va[0], va[1], va[2], col);
			dd->vertex(con->pos[0], con->pos[1], con->pos[2], col);
			col2 = startSet ? col : duRGBA(220, 32, 16, 196);
			duAppendCircle(dd, con->pos[0], con->pos[1] + 0.1f, con->pos[2], con->rad, col2);

			dd->vertex(vb[0], vb[1], vb[2], col);
			dd->vertex(con->pos[3], con->pos[4], con->pos[5], col);
			col2 = endSet ? col : duRGBA(220, 32, 16, 196);
			duAppendCircle(dd, con->pos[3], con->pos[4] + 0.1f, con->pos[5], con->rad, col2);

			// End point vertices.
			dd->vertex(con->pos[0], con->pos[1], con->pos[2], duRGBA(0, 48, 64, 196));
			dd->vertex(con->pos[0], con->pos[1] + 0.2f, con->pos[2], duRGBA(0, 48, 64, 196));

			dd->vertex(con->pos[3], con->pos[4], con->pos[5], duRGBA(0, 48, 64, 196));
			dd->vertex(con->pos[3], con->pos[4] + 0.2f, con->pos[5], duRGBA(0, 48, 64, 196));

			// Connection arc.
			duAppendArc(dd, con->pos[0], con->pos[1], con->pos[2], con->pos[3], con->pos[4], con->pos[5], 0.25f,
				(con->flags & 1) ? 0.6f : 0, 0.6f, col);
		}
		dd->end();
	}

	dd->depthMask(true);
}

void duDebugDrawNavMeshx(duDebugDraw* dd, const dtNavMesh& mesh, unsigned char flags)
{
	if (!dd) return;

	for (int i = 0; i < mesh.getMaxTiles(); ++i)
	{
		const dtMeshTile* tile = mesh.getTile(i);
		if (!tile->header) continue;
		drawMeshTilex(dd, mesh, 0, tile, flags);
	}
}

void NavigationManager::render()
{
	// when loading from a navmesh file, no m_geom is loaded
	bool navmesh_from_disk = false;
	if (!m_geom || !m_geom->getMesh())
	{
		navmesh_from_disk = true;
		//return;
	}

	DebugDrawGL dd;

	const float texScale = 1.0f / (m_cellSize * 10.0f);

	if (!navmesh_from_disk)
	{

		if (m_drawMode != DRAWMODE_NAVMESH_TRANS &&
			NavSysDebug::ShowOffmeshConnections &&
			NavSysDebug::RedrawOffmeshConnections)
		{
			NavSysDebug::DebugObject = NavSysDebug::OffmeshConectionsObject;
			NavSysDebug::DebugObject->clear();
			m_geom->drawOffMeshConnections(&dd);
			NavSysDebug::RedrawOffmeshConnections = false;
		}

		// Draw bounds
		const float* bmin = m_geom->getMeshBoundsMin();
		const float* bmax = m_geom->getMeshBoundsMax();

		if (NavSysDebug::ShowBounds && NavSysDebug::RedrawBounds)
		{
			NavSysDebug::DebugObject = NavSysDebug::BoundsObject;
			NavSysDebug::DebugObject->clear();
			duDebugDrawBoxWire(&dd, bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2], duRGBA(255, 255, 255, 128), 1.0f);
			NavSysDebug::RedrawBounds = false;
		}

		if (NavSysDebug::ShowTilingGrid && NavSysDebug::RedrawTilingGrid)
		{
			// Tiling grid.
			int gw = 0, gh = 0;
			rcCalcGridSize(bmin, bmax, m_cellSize, &gw, &gh);
			const int tw = (gw + (int)m_tileSize - 1) / (int)m_tileSize;
			const int th = (gh + (int)m_tileSize - 1) / (int)m_tileSize;
			const float s = m_tileSize*m_cellSize;

			NavSysDebug::DebugObject = NavSysDebug::TilingGridObject;
			NavSysDebug::DebugObject->clear();
			duDebugDrawGridXZ(&dd, bmin[0], bmin[1], bmin[2], tw, th, s, duRGBA(0, 0, 0, 64), 1.0f);
		}

		// TODO
		if (NavSysDebug::ShowConvexVolumes && NavSysDebug::RedrawConvexVolumes)
		{
			NavSysDebug::DebugObject = NavSysDebug::ConvexVolumeObjects;
			NavSysDebug::DebugObject->clear();

			m_geom->drawConvexVolumes(&dd);
			
			// temporary convex volume
			drawConvexVolume(&dd);

			NavSysDebug::RedrawConvexVolumes = false;
		}

		m_crowdTool->render();
	}

	//if (m_tileCache && m_drawMode == DRAWMODE_CACHE_BOUNDS)
	//	drawTiles(&dd, m_tileCache);

	if (m_tileCache && NavSysDebug::ShowObstacles && NavSysDebug::RedrawObstacles)
	{
		NavSysDebug::DebugObject = NavSysDebug::ObstaclesObject;
		NavSysDebug::DebugObject->clear();
		drawObstacles(&dd, m_tileCache);
		NavSysDebug::RedrawObstacles = false;
	}

	if (NavSysDebug::ShowNavmesh && NavSysDebug::RedrawNavmesh)
	{
		NavSysDebug::DebugObject = NavSysDebug::NavmeshObject;
		NavSysDebug::DebugObject->clear();

		if (m_navMesh && m_navQuery &&
			(m_drawMode == DRAWMODE_NAVMESH ||
			 m_drawMode == DRAWMODE_NAVMESH_TRANS ||
			 m_drawMode == DRAWMODE_NAVMESH_BVTREE ||
			 m_drawMode == DRAWMODE_NAVMESH_NODES ||
			 m_drawMode == DRAWMODE_NAVMESH_PORTALS ||
			 m_drawMode == DRAWMODE_NAVMESH_INVIS))
		{
			if (m_drawMode != DRAWMODE_NAVMESH_INVIS)
				//--duDebugDrawNavMeshWithClosedList(&dd, *m_navMesh, *m_navQuery, m_navMeshDrawFlags|DU_DRAWNAVMESH_COLOR_TILES);
				//++
				duDebugDrawNavMeshx(&dd, *m_navMesh, m_navMeshDrawFlags | DU_DRAWNAVMESH_COLOR_TILES);
			/*
			if (m_drawMode == DRAWMODE_NAVMESH_BVTREE)
				duDebugDrawNavMeshBVTree(&dd, *m_navMesh);
			if (m_drawMode == DRAWMODE_NAVMESH_PORTALS)
				duDebugDrawNavMeshPortals(&dd, *m_navMesh);
			if (m_drawMode == DRAWMODE_NAVMESH_NODES)
				duDebugDrawNavMeshNodes(&dd, *m_navQuery);
			*/
			//duDebugDrawNavMeshPolysWithFlags(&dd, *m_navMesh, SAMPLE_POLYFLAGS_DISABLED, duRGBA(0,0,0,128));
		}

		NavSysDebug::RedrawNavmesh = false;
	}
}

void NavigationManager::renderCachedTile(const int tx, const int ty, const int type)
{
	DebugDrawGL dd;
	if (m_tileCache)
		drawDetail(&dd,m_tileCache,tx,ty,type);
}

void NavigationManager::renderCachedTileOverlay(const int tx, const int ty, double* proj, double* model, int* view)
{
	if (m_tileCache)
		drawDetailOverlay(m_tileCache, tx, ty, proj, model, view);
}

void NavigationManager::changeMesh(class InputGeom* geom)
{
	m_geom = geom;

	dtFreeTileCache(m_tileCache);
	m_tileCache = 0;
	
	dtFreeNavMesh(m_navMesh);
	m_navMesh = 0;
}

void NavigationManager::addTempObstacle(const float* pos)
{
	if (!m_tileCache)
		return;
	float p[3];
	dtVcopy(p, pos);
	p[1] -= 0.5f;
	m_tileCache->addObstacle(p, 1.0f, 2.0f, 0);
}

void NavigationManager::removeTempObstacle(const float* sp, const float* sq)
{
	if (!m_tileCache)
		return;
	dtObstacleRef ref = hitTestObstacle(m_tileCache, sp, sq);
	m_tileCache->removeObstacle(ref);
}

void NavigationManager::clearAllTempObstacles()
{
	if (!m_tileCache)
		return;
	for (int i = 0; i < m_tileCache->getObstacleCount(); ++i)
	{
		const dtTileCacheObstacle* ob = m_tileCache->getObstacle(i);
		if (ob->state == DT_OBSTACLE_EMPTY) continue;
		m_tileCache->removeObstacle(m_tileCache->getObstacleRef(ob));
	}
}

bool NavigationManager::build()
{
	dtStatus status;

	if (!m_geom || !m_geom->getMesh())
	{
		m_ctx->log(RC_LOG_ERROR, "buildTiledNavigation: No vertices and triangles.");
		return false;
	}

	m_tmproc->init(m_geom);
	
	// Init cache
	const float* bmin = m_geom->getMeshBoundsMin();
	const float* bmax = m_geom->getMeshBoundsMax();
	int gw = 0, gh = 0;
	rcCalcGridSize(bmin, bmax, m_cellSize, &gw, &gh);
	const int ts = (int)m_tileSize;
	const int tw = (gw + ts-1) / ts;
	const int th = (gh + ts-1) / ts;

	// Generation params.
	rcConfig cfg;
	memset(&cfg, 0, sizeof(cfg));
	cfg.cs = m_cellSize;
	cfg.ch = m_cellHeight;
	cfg.walkableSlopeAngle = m_agentMaxSlope;
	cfg.walkableHeight = (int)ceilf(m_agentHeight / cfg.ch);
	cfg.walkableClimb = (int)floorf(m_agentMaxClimb / cfg.ch);
	cfg.walkableRadius = (int)ceilf(m_agentRadius / cfg.cs);
	cfg.maxEdgeLen = (int)(m_edgeMaxLen / m_cellSize);
	cfg.maxSimplificationError = m_edgeMaxError;
	cfg.minRegionArea = (int)rcSqr(m_regionMinSize);		// Note: area = size*size
	cfg.mergeRegionArea = (int)rcSqr(m_regionMergeSize);	// Note: area = size*size
	cfg.maxVertsPerPoly = (int)m_vertsPerPoly;
	cfg.tileSize = (int)m_tileSize;
	cfg.borderSize = cfg.walkableRadius + 3; // Reserve enough padding.
	cfg.width = cfg.tileSize + cfg.borderSize*2;
	cfg.height = cfg.tileSize + cfg.borderSize*2;
	cfg.detailSampleDist = m_detailSampleDist < 0.9f ? 0 : m_cellSize * m_detailSampleDist;
	cfg.detailSampleMaxError = m_cellHeight * m_detailSampleMaxError;
	rcVcopy(cfg.bmin, bmin);
	rcVcopy(cfg.bmax, bmax);
	
	// Tile cache params.
	dtTileCacheParams tcparams;
	memset(&tcparams, 0, sizeof(tcparams));
	rcVcopy(tcparams.orig, bmin);
	tcparams.cs = m_cellSize;
	tcparams.ch = m_cellHeight;
	tcparams.width = (int)m_tileSize;
	tcparams.height = (int)m_tileSize;
	tcparams.walkableHeight = m_agentHeight;
	tcparams.walkableRadius = m_agentRadius;
	tcparams.walkableClimb = m_agentMaxClimb;
	tcparams.maxSimplificationError = m_edgeMaxError;
	tcparams.maxTiles = tw*th*EXPECTED_LAYERS_PER_TILE;
	tcparams.maxObstacles = 128;

	dtFreeTileCache(m_tileCache);
	
	m_tileCache = dtAllocTileCache();
	if (!m_tileCache)
	{
		m_ctx->log(RC_LOG_ERROR, "buildTiledNavigation: Could not allocate tile cache.");
		return false;
	}
	status = m_tileCache->init(&tcparams, m_talloc, m_tcomp, m_tmproc);
	if (dtStatusFailed(status))
	{
		m_ctx->log(RC_LOG_ERROR, "buildTiledNavigation: Could not init tile cache.");
		return false;
	}
	
	dtFreeNavMesh(m_navMesh);
	
	m_navMesh = dtAllocNavMesh();
	if (!m_navMesh)
	{
		m_ctx->log(RC_LOG_ERROR, "buildTiledNavigation: Could not allocate navmesh.");
		return false;
	}

	dtNavMeshParams params;
	memset(&params, 0, sizeof(params));
	rcVcopy(params.orig, m_geom->getMeshBoundsMin());
	params.tileWidth = m_tileSize*m_cellSize;
	params.tileHeight = m_tileSize*m_cellSize;
	params.maxTiles = m_maxTiles;
	params.maxPolys = m_maxPolysPerTile;
	
	status = m_navMesh->init(&params);
	if (dtStatusFailed(status))
	{
		m_ctx->log(RC_LOG_ERROR, "buildTiledNavigation: Could not init navmesh.");
		return false;
	}
	
	status = m_navQuery->init(m_navMesh, 2048);
	if (dtStatusFailed(status))
	{
		m_ctx->log(RC_LOG_ERROR, "buildTiledNavigation: Could not init Detour navmesh query");
		return false;
	}

	// Preprocess tiles.
	
	m_ctx->resetTimers();
	
	m_cacheLayerCount = 0;
	m_cacheCompressedSize = 0;
	m_cacheRawSize = 0;
	
	for (int y = 0; y < th; ++y)
	{
		for (int x = 0; x < tw; ++x)
		{
			TileCacheData tiles[MAX_LAYERS];
			memset(tiles, 0, sizeof(tiles));
			int ntiles = rasterizeTileLayers(m_ctx, m_geom, x, y, cfg, tiles, MAX_LAYERS);

			for (int i = 0; i < ntiles; ++i)
			{
				TileCacheData* tile = &tiles[i];
				status = m_tileCache->addTile(tile->data, tile->dataSize, DT_COMPRESSEDTILE_FREE_DATA, 0);
				if (dtStatusFailed(status))
				{
					dtFree(tile->data);
					tile->data = 0;
					continue;
				}
				
				m_cacheLayerCount++;
				m_cacheCompressedSize += tile->dataSize;
				m_cacheRawSize += calcLayerBufferSize(tcparams.width, tcparams.height);
			}
		}
	}

	// Build initial meshes
	m_ctx->startTimer(RC_TIMER_TOTAL);
	for (int y = 0; y < th; ++y)
		for (int x = 0; x < tw; ++x)
			m_tileCache->buildNavMeshTilesAt(x,y, m_navMesh);
	m_ctx->stopTimer(RC_TIMER_TOTAL);
	
	m_cacheBuildTimeMs = m_ctx->getAccumulatedTime(RC_TIMER_TOTAL)/1000.0f;
	m_cacheBuildMemUsage = m_talloc->high;
	
	//--
	/*
	const dtNavMesh* nav = m_navMesh;
	int navmeshMemUsage = 0;
	for (int i = 0; i < nav->getMaxTiles(); ++i)
	{
		const dtMeshTile* tile = nav->getTile(i);
		if (tile->header)
			navmeshMemUsage += tile->dataSize;
	}
	printf("navmeshMemUsage = %.1f kB", navmeshMemUsage/1024.0f);
	*/
	m_crowdTool->init(this);

	return true;
}

void NavigationManager::update(const float dt)
{
	m_crowdTool->update(dt);

	if (!m_navMesh)
		return;
	if (!m_tileCache)
		return;
	
	m_tileCache->update(dt, m_navMesh);
}

void NavigationManager::getTilePos(const float* pos, int& tx, int& ty)
{
	if (!m_geom) return;
	
	const float* bmin = m_geom->getMeshBoundsMin();
	
	const float ts = m_tileSize*m_cellSize;
	tx = (int)((pos[0] - bmin[0]) / ts);
	ty = (int)((pos[2] - bmin[2]) / ts);
}

static const int TILECACHESET_MAGIC = 'T'<<24 | 'S'<<16 | 'E'<<8 | 'T'; //'TSET';
static const int TILECACHESET_VERSION = 1;

struct TileCacheSetHeader
{
	int magic;
	int version;
	int numTiles;
	dtNavMeshParams meshParams;
	dtTileCacheParams cacheParams;
};

struct TileCacheTileHeader
{
	dtCompressedTileRef tileRef;
	int dataSize;
};

bool NavigationManager::saveAll(const char* path)
{
	if (!m_tileCache) return false;
	
	FILE* fp = fopen(path, "wb");
	if (!fp)
		return false;
	
	// Store header.
	TileCacheSetHeader header;
	header.magic = TILECACHESET_MAGIC;
	header.version = TILECACHESET_VERSION;
	header.numTiles = 0;
	for (int i = 0; i < m_tileCache->getTileCount(); ++i)
	{
		const dtCompressedTile* tile = m_tileCache->getTile(i);
		if (!tile || !tile->header || !tile->dataSize) continue;
		header.numTiles++;
	}
	memcpy(&header.cacheParams, m_tileCache->getParams(), sizeof(dtTileCacheParams));
	memcpy(&header.meshParams, m_navMesh->getParams(), sizeof(dtNavMeshParams));
	fwrite(&header, sizeof(TileCacheSetHeader), 1, fp);

	// Store tiles.
	for (int i = 0; i < m_tileCache->getTileCount(); ++i)
	{
		const dtCompressedTile* tile = m_tileCache->getTile(i);
		if (!tile || !tile->header || !tile->dataSize) continue;

		TileCacheTileHeader tileHeader;
		tileHeader.tileRef = m_tileCache->getTileRef(tile);
		tileHeader.dataSize = tile->dataSize;
		fwrite(&tileHeader, sizeof(tileHeader), 1, fp);

		fwrite(tile->data, tile->dataSize, 1, fp);
	}

	fclose(fp);
	return true;
}

bool NavigationManager::loadAll(const char* path)
{
	FILE* fp = fopen(path, "rb");
	if (!fp) return false;

	// Read header.
	TileCacheSetHeader header;
	fread(&header, sizeof(TileCacheSetHeader), 1, fp);
	if (header.magic != TILECACHESET_MAGIC)
	{
		fclose(fp);
		return false;
	}
	if (header.version != TILECACHESET_VERSION)
	{
		fclose(fp);
		return false;
	}

	m_navMesh = dtAllocNavMesh();
	if (!m_navMesh)
	{
		fclose(fp);
		return false;
	}
	dtStatus status = m_navMesh->init(&header.meshParams);
	if (dtStatusFailed(status))
	{
		fclose(fp);
		return false;
	}

	m_tileCache = dtAllocTileCache();
	if (!m_tileCache)
	{
		fclose(fp);
		return false;
	}
	status = m_tileCache->init(&header.cacheParams, m_talloc, m_tcomp, m_tmproc);
	if (dtStatusFailed(status))
	{
		fclose(fp);
		return false;
	}

	// Read tiles.
	for (int i = 0; i < header.numTiles; ++i)
	{
		TileCacheTileHeader tileHeader;
		fread(&tileHeader, sizeof(tileHeader), 1, fp);
		if (!tileHeader.tileRef || !tileHeader.dataSize)
			break;

		unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
		if (!data) break;
		memset(data, 0, tileHeader.dataSize);
		fread(data, tileHeader.dataSize, 1, fp);
		
		dtCompressedTileRef tile = 0;
		m_tileCache->addTile(data, tileHeader.dataSize, DT_COMPRESSEDTILE_FREE_DATA, &tile);

		if (tile)
			m_tileCache->buildNavMeshTile(tile, m_navMesh);
	}

	fclose(fp);
	return true;
}

void NavigationManager::resetCommonSettings()
{
	m_cellSize = 0.3f;
	m_cellHeight = 0.2f;
	m_agentHeight = 2.0f;
	m_agentRadius = 0.6f;
	m_agentMaxClimb = 0.9f;
	m_agentMaxSlope = 45.0f;
	m_regionMinSize = 8;
	m_regionMergeSize = 20;
	m_edgeMaxLen = 12.0f;
	m_edgeMaxError = 1.3f;
	m_vertsPerPoly = 6.0f;
	m_detailSampleDist = 6.0f;
	m_detailSampleMaxError = 1.0f;
	m_partitionType = SAMPLE_PARTITION_WATERSHED;
}

const float* NavigationManager::getBoundsMin()
{
	if (!m_geom) return 0;
	return m_geom->getMeshBoundsMin();
}

const float* NavigationManager::getBoundsMax()
{
	if (!m_geom) return 0;
	return m_geom->getMeshBoundsMax();
}

void NavigationManager::step()
{
	
}

bool NavigationManager::load(const char* path)
{
	updateMaxTiles();
	dtFreeNavMesh(m_navMesh);
	dtFreeTileCache(m_tileCache);
	bool result = loadAll(path);
	m_navQuery->init(m_navMesh, 2048);

	m_crowdTool->init(this);
	return result;
}

void NavigationManager::freeNavmesh()
{
	dtFreeTileCache(m_tileCache);
	m_tileCache = 0;

	dtFreeNavMesh(m_navMesh);
	m_navMesh = 0;
}

// CONVEX VOLUMES: (from Convex Volume Tool)
// Returns true if 'c' is left of line 'a'-'b'.
inline bool left(const float* a, const float* b, const float* c)
{
	const float u1 = b[0] - a[0];
	const float v1 = b[2] - a[2];
	const float u2 = c[0] - a[0];
	const float v2 = c[2] - a[2];
	return u1 * v2 - v1 * u2 < 0;
}

// Returns true if 'a' is more lower-left than 'b'.
inline bool cmppt(const float* a, const float* b)
{
	if (a[0] < b[0]) return true;
	if (a[0] > b[0]) return false;
	if (a[2] < b[2]) return true;
	if (a[2] > b[2]) return false;
	return false;
}
// Calculates convex hull on xz-plane of points on 'pts',
// stores the indices of the resulting hull in 'out' and
// returns number of points on hull.
static int convexhull(const float* pts, int npts, int* out)
{
	// Find lower-leftmost point.
	int hull = 0;
	for (int i = 1; i < npts; ++i)
	if (cmppt(&pts[i * 3], &pts[hull * 3]))
		hull = i;
	// Gift wrap hull.
	int endpt = 0;
	int i = 0;
	do
	{
		out[i++] = hull;
		endpt = 0;
		for (int j = 1; j < npts; ++j)
		if (hull == endpt || left(&pts[hull * 3], &pts[endpt * 3], &pts[j * 3]))
			endpt = j;
		hull = endpt;
	} while (endpt != out[0]);

	return i;
}

static int pointInPoly(int nvert, const float* verts, const float* p)
{
	int i, j, c = 0;
	for (i = 0, j = nvert - 1; i < nvert; j = i++)
	{
		const float* vi = &verts[i * 3];
		const float* vj = &verts[j * 3];
		if (((vi[2] > p[2]) != (vj[2] > p[2])) &&
			(p[0] < (vj[0] - vi[0]) * (p[2] - vi[2]) / (vj[2] - vi[2]) + vi[0]))
			c = !c;
	}
	return c;
}

void NavigationManager::removeConvexVolume(float* p)
{
	if (!m_geom || !m_geom->getMesh())
	{
		return;
	}
	int nearestIndex = -1;
	const ConvexVolume* vols = m_geom->getConvexVolumes();
	for (int i = 0; i < m_geom->getConvexVolumeCount(); ++i)
	{
		if (pointInPoly(vols[i].nverts, vols[i].verts, p) &&
			p[1] >= vols[i].hmin && p[1] <= vols[i].hmax)
		{
			nearestIndex = i;
		}
	}
	// If end point close enough, delete it.
	if (nearestIndex != -1)
	{
		m_geom->deleteConvexVolume(nearestIndex);
	}
}

void NavigationManager::createConvexVolume(float* p)
{
	if (!m_geom || !m_geom->getMesh())
	{
		return;
	}
	// If clicked on that last pt, create the shape.
	if (m_npts && rcVdistSqr(p, &m_pts[(m_npts - 1) * 3]) < rcSqr(0.2f))
	{
		if (m_nhull > 2)
		{
			// Create shape.
			float verts[MAX_PTS * 3];
			for (int i = 0; i < m_nhull; ++i)
				rcVcopy(&verts[i * 3], &m_pts[m_hull[i] * 3]);

			float minh = FLT_MAX, maxh = 0;
			for (int i = 0; i < m_nhull; ++i)
				minh = rcMin(minh, verts[i * 3 + 1]);
			minh -= m_boxDescent;
			maxh = minh + m_boxHeight;

			if (m_polyOffset > 0.01f)
			{
				float offset[MAX_PTS * 2 * 3];
				int noffset = rcOffsetPoly(verts, m_nhull, m_polyOffset, offset, MAX_PTS * 2);
				if (noffset > 0)
					m_geom->addConvexVolume(offset, noffset, minh, maxh, (unsigned char)m_areaType);
			}
			else
			{
				m_geom->addConvexVolume(verts, m_nhull, minh, maxh, (unsigned char)m_areaType);
			}
		}

		m_npts = 0;
		m_nhull = 0;
	}
	else
	{
		// Add new point 
		if (m_npts < MAX_PTS)
		{
			rcVcopy(&m_pts[m_npts * 3], p);
			m_npts++;
			// Update hull.
			if (m_npts > 1)
				m_nhull = convexhull(m_pts, m_npts, m_hull);
			else
				m_nhull = 0;
		}
	}
}

// Temporary convex volume
void NavigationManager::drawConvexVolume(DebugDrawGL* dd)
{
	// Find height extents of the shape.
	float minh = FLT_MAX, maxh = 0;
	for (int i = 0; i < m_npts; ++i)
		minh = rcMin(minh, m_pts[i * 3 + 1]);
	minh -= m_boxDescent;
	maxh = minh + m_boxHeight;

	dd->begin(DU_DRAW_POINTS, 4.0f);
	for (int i = 0; i < m_npts; ++i)
	{
		unsigned int col = duRGBA(255, 255, 255, 255);
		if (i == m_npts - 1)
			col = duRGBA(240, 32, 16, 255);
		dd->vertex(m_pts[i * 3 + 0], m_pts[i * 3 + 1] + 0.1f, m_pts[i * 3 + 2], col);
	}
	dd->end();

	dd->begin(DU_DRAW_LINES, 2.0f);
	for (int i = 0, j = m_nhull - 1; i < m_nhull; j = i++)
	{
		const float* vi = &m_pts[m_hull[j] * 3];
		const float* vj = &m_pts[m_hull[i] * 3];
		dd->vertex(vj[0], minh, vj[2], duRGBA(255, 255, 255, 64));
		dd->vertex(vi[0], minh, vi[2], duRGBA(255, 255, 255, 64));
		dd->vertex(vj[0], maxh, vj[2], duRGBA(255, 255, 255, 64));
		dd->vertex(vi[0], maxh, vi[2], duRGBA(255, 255, 255, 64));
		dd->vertex(vj[0], minh, vj[2], duRGBA(255, 255, 255, 64));
		dd->vertex(vj[0], maxh, vj[2], duRGBA(255, 255, 255, 64));
	}
	dd->end();
}