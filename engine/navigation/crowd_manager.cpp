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

// This is a modified version of CrowdTool.cpp from Recast Demo

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include "crowd_manager.h"
#include "input_geom.h"
#include "navigation_manager.h"
#include "DetourCrowd.h"
#include "DetourDebugDraw.h"
#include "DetourObstacleAvoidance.h"
#include "DetourCommon.h"
#include "DetourNode.h"
#include "navigation_interfaces.h"

#include"navigation_system.h"

static bool isectSegAABB(const float* sp, const float* sq,
						 const float* amin, const float* amax,
						 float& tmin, float& tmax)
{
	static const float EPS = 1e-6f;
	
	float d[3];
	dtVsub(d, sq, sp);
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
			if (t1 > t2) dtSwap(t1, t2);
			// Compute the intersection of slab intersections intervals
			if (t1 > tmin) tmin = t1;
			if (t2 < tmax) tmax = t2;
			// Exit with no collision as soon as slab intersection becomes empty
			if (tmin > tmax) return false;
		}
	}
	
	return true;
}

static void getAgentBounds(const dtCrowdAgent* ag, float* bmin, float* bmax)
{
	const float* p = ag->npos;
	const float r = ag->params.radius;
	const float h = ag->params.height;
	bmin[0] = p[0] - r;
	bmin[1] = p[1];
	bmin[2] = p[2] - r;
	bmax[0] = p[0] + r;
	bmax[1] = p[1] + h;
	bmax[2] = p[2] + r;
}

int CrowdTool::addAgent(const float* p)
{
	if (!m_nvmgr) return -1;
	dtCrowd* crowd = m_nvmgr->getCrowd();

	dtCrowdAgentParams ap;
	memset(&ap, 0, sizeof(ap));
	ap.radius = m_nvmgr->getAgentRadius();
	ap.height = m_nvmgr->getAgentHeight();
	ap.maxAcceleration = 8.0f;
	ap.maxSpeed = 3.5f;
	ap.collisionQueryRange = ap.radius * 12.0f;
	ap.pathOptimizationRange = ap.radius * 30.0f;
	ap.updateFlags = 0; 
	if (m_toolParams.m_anticipateTurns)
		ap.updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
	if (m_toolParams.m_optimizeVis)
		ap.updateFlags |= DT_CROWD_OPTIMIZE_VIS;
	if (m_toolParams.m_optimizeTopo)
		ap.updateFlags |= DT_CROWD_OPTIMIZE_TOPO;
	if (m_toolParams.m_obstacleAvoidance)
		ap.updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
	if (m_toolParams.m_separation)
		ap.updateFlags |= DT_CROWD_SEPARATION;
	ap.obstacleAvoidanceType = (unsigned char)m_toolParams.m_obstacleAvoidanceType;
	ap.separationWeight = m_toolParams.m_separationWeight;
	
	int idx = crowd->addAgent(p, &ap);
	//--
	//if (idx != -1)
	//{
	//	if (m_targetRef)
	//		crowd->requestMoveTarget(idx, m_targetRef, m_targetPos);
	//}
	return idx;
}

void CrowdTool::removeAgent(const int idx)
{
	if (!m_nvmgr) return;
	dtCrowd* crowd = m_nvmgr->getCrowd();

	crowd->removeAgent(idx);
	
	if (idx == m_agentDebug.idx)
		m_agentDebug.idx = -1;
}

void CrowdTool::hilightAgent(const int idx)
{
	m_agentDebug.idx = idx;
}

static void calcVel(float* vel, const float* pos, const float* tgt, const float speed)
{
	dtVsub(vel, tgt, pos);
	vel[1] = 0.0;
	dtVnormalize(vel);
	dtVscale(vel, vel, speed);
}

void CrowdTool::setMoveTarget(const float* p, bool adjust)
{
	if (!m_nvmgr) return;
	
	// Find nearest point on navmesh and set move request to that location.
	dtNavMeshQuery* navquery = m_nvmgr->getNavMeshQuery();
	dtCrowd* crowd = m_nvmgr->getCrowd();
	const dtQueryFilter* filter = crowd->getFilter(0);
	const float* ext = crowd->getQueryExtents();

	if (adjust)
	{
		float vel[3];
		// Request velocity
		if (m_agentDebug.idx != -1)
		{
			const dtCrowdAgent* ag = crowd->getAgent(m_agentDebug.idx);
			if (ag && ag->active)
			{
				calcVel(vel, ag->npos, p, ag->params.maxSpeed);
				crowd->requestMoveVelocity(m_agentDebug.idx, vel);
			}
		}
		else
		{
			for (int i = 0; i < crowd->getAgentCount(); ++i)
			{
				const dtCrowdAgent* ag = crowd->getAgent(i);
				if (!ag->active) continue;
				calcVel(vel, ag->npos, p, ag->params.maxSpeed);
				crowd->requestMoveVelocity(i, vel);
			}
		}
	}
	else
	{
		navquery->findNearestPoly(p, ext, filter, &m_targetRef, m_targetPos);
		
		if (m_agentDebug.idx != -1)
		{
			const dtCrowdAgent* ag = crowd->getAgent(m_agentDebug.idx);
			if (ag && ag->active)
				crowd->requestMoveTarget(m_agentDebug.idx, m_targetRef, m_targetPos);
		}
		else
		{
			for (int i = 0; i < crowd->getAgentCount(); ++i)
			{
				const dtCrowdAgent* ag = crowd->getAgent(i);
				if (!ag->active) continue;
				crowd->requestMoveTarget(i, m_targetRef, m_targetPos);
			}
		}
	}
}

int CrowdTool::hitTestAgents(const float* s, const float* p)
{
	if (!m_nvmgr) return -1;
	dtCrowd* crowd = m_nvmgr->getCrowd();
	
	int isel = -1;
	float tsel = FLT_MAX;

	for (int i = 0; i < crowd->getAgentCount(); ++i)
	{
		const dtCrowdAgent* ag = crowd->getAgent(i);
		if (!ag->active) continue;
		float bmin[3], bmax[3];
		getAgentBounds(ag, bmin, bmax);
		float tmin, tmax;
		if (isectSegAABB(s, p, bmin,bmax, tmin, tmax))
		{
			if (tmin > 0 && tmin < tsel)
			{
				isel = i;
				tsel = tmin;
			} 
		}
	}

	return isel;
}

void CrowdTool::updateAgentParams()
{
	if (!m_nvmgr) return;
	dtCrowd* crowd = m_nvmgr->getCrowd();
	if (!crowd) return;
	
	unsigned char updateFlags = 0;
	unsigned char obstacleAvoidanceType = 0;
	
	if (m_toolParams.m_anticipateTurns)
		updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
	if (m_toolParams.m_optimizeVis)
		updateFlags |= DT_CROWD_OPTIMIZE_VIS;
	if (m_toolParams.m_optimizeTopo)
		updateFlags |= DT_CROWD_OPTIMIZE_TOPO;
	if (m_toolParams.m_obstacleAvoidance)
		updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
	if (m_toolParams.m_obstacleAvoidance)
		updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
	if (m_toolParams.m_separation)
		updateFlags |= DT_CROWD_SEPARATION;
	
	obstacleAvoidanceType = (unsigned char)m_toolParams.m_obstacleAvoidanceType;
	
	dtCrowdAgentParams params;
	
	for (int i = 0; i < crowd->getAgentCount(); ++i)
	{
		const dtCrowdAgent* ag = crowd->getAgent(i);
		if (!ag->active) continue;
		memcpy(&params, &ag->params, sizeof(dtCrowdAgentParams));
		params.updateFlags = updateFlags;
		params.obstacleAvoidanceType = obstacleAvoidanceType;
		params.separationWeight = m_toolParams.m_separationWeight;
		crowd->updateAgentParameters(i, &params);
	}	
}

void CrowdTool::updateTick(const float dt)
{
	if (!m_nvmgr) return;
	dtNavMesh* nav = m_nvmgr->getNavMesh();
	dtCrowd* crowd = m_nvmgr->getCrowd();
	if (!nav || !crowd) return;

	crowd->update(dt, &m_agentDebug);
	m_agentDebug.vod->normalizeSamples();
}

CrowdTool::CrowdTool() :
	m_nvmgr(0),
	m_mode(TOOLMODE_CREATE),
	m_nav(0),
	m_crowd(0),
	m_targetRef(0)
{
	m_toolParams.m_expandSelectedDebugDraw = true;
	m_toolParams.m_showCorners = false;
	m_toolParams.m_showCollisionSegments = false;
	m_toolParams.m_showPath = false;
	m_toolParams.m_showVO = false;
	m_toolParams.m_showOpt = false;
	m_toolParams.m_showNeis = false;
	m_toolParams.m_expandDebugDraw = false;
	m_toolParams.m_showLabels = false;
	m_toolParams.m_showGrid = false;
	m_toolParams.m_showNodes = false;
	m_toolParams.m_showPerfGraph = false;
	m_toolParams.m_showDetailAll = false;
	m_toolParams.m_expandOptions = true;
	m_toolParams.m_anticipateTurns = true;
	m_toolParams.m_optimizeVis = true;
	m_toolParams.m_optimizeTopo = true;
	m_toolParams.m_obstacleAvoidance = true;
	m_toolParams.m_obstacleAvoidanceType = 3.0f;
	m_toolParams.m_separation = false;
	m_toolParams.m_separationWeight = 2.0f;

	m_vod = dtAllocObstacleAvoidanceDebugData();
	m_vod->init(2048);

	memset(&m_agentDebug, 0, sizeof(m_agentDebug));
	m_agentDebug.idx = -1;
	m_agentDebug.vod = m_vod;
}

CrowdTool::~CrowdTool()
{
	dtFreeObstacleAvoidanceDebugData(m_vod);
}

void CrowdTool::init(NavigationManager* nvmgr)
{
	if (m_nvmgr != nvmgr)
	{
		m_nvmgr = nvmgr;
		//		m_oldFlags = m_nvmgr->getNavMeshDrawFlags();
		//		m_nvmgr->setNavMeshDrawFlags(m_oldFlags & ~DU_DRAWNAVMESH_CLOSEDLIST);
	}
	
	if (!nvmgr)
		return;

	dtNavMesh* nav = m_nvmgr->getNavMesh();
	dtCrowd* crowd = m_nvmgr->getCrowd();

	if (nav && crowd && (m_nav != nav || m_crowd != crowd))
	{
		m_nav = nav;
		m_crowd = crowd;

		crowd->init(MAX_AGENTS, m_nvmgr->getAgentRadius(), nav);

		// Make polygons with 'disabled' flag invalid.
		crowd->getEditableFilter(0)->setExcludeFlags(SAMPLE_POLYFLAGS_DISABLED);

		// Setup local avoidance params to different qualities.
		dtObstacleAvoidanceParams params;
		// Use mostly default settings, copy from dtCrowd.
		memcpy(&params, crowd->getObstacleAvoidanceParams(0), sizeof(dtObstacleAvoidanceParams));

		// Low (11)
		params.velBias = 0.5f;
		params.adaptiveDivs = 5;
		params.adaptiveRings = 2;
		params.adaptiveDepth = 1;
		crowd->setObstacleAvoidanceParams(0, &params);

		// Medium (22)
		params.velBias = 0.5f;
		params.adaptiveDivs = 5;
		params.adaptiveRings = 2;
		params.adaptiveDepth = 2;
		crowd->setObstacleAvoidanceParams(1, &params);

		// Good (45)
		params.velBias = 0.5f;
		params.adaptiveDivs = 7;
		params.adaptiveRings = 2;
		params.adaptiveDepth = 3;
		crowd->setObstacleAvoidanceParams(2, &params);

		// High (66)
		params.velBias = 0.5f;
		params.adaptiveDivs = 7;
		params.adaptiveRings = 3;
		params.adaptiveDepth = 3;

		crowd->setObstacleAvoidanceParams(3, &params);
	}
}

void CrowdTool::reset()
{
}

void CrowdTool::step()
{
	const float dt = 1.0f/20.0f;

	updateTick(dt);
}

void CrowdTool::update(const float dt)
{
	rcIgnoreUnused(dt);
	updateTick(dt);
}

void CrowdTool::render()
{
	DebugDrawGL dd;
	const float rad = m_nvmgr->getAgentRadius();

	dtNavMesh* nav = m_nvmgr->getNavMesh();
	dtCrowd* crowd = m_nvmgr->getCrowd();
	if (!nav || !crowd)
		return;

	if (m_toolParams.m_showNodes && crowd->getPathQueue())
	{
		const dtNavMeshQuery* navquery = crowd->getPathQueue()->getNavQuery();
		if (navquery)
			duDebugDrawNavMeshNodes(&dd, *navquery);
	}

	// Draw paths
	if (m_toolParams.m_showPath)
	{
		for (int i = 0; i < crowd->getAgentCount(); i++)
		{
			if (m_toolParams.m_showDetailAll == false && i != m_agentDebug.idx)
				continue;
			const dtCrowdAgent* ag = crowd->getAgent(i);
			if (!ag->active)
				continue;
			const dtPolyRef* path = ag->corridor.getPath();
			const int npath = ag->corridor.getPathCount();
			for (int j = 0; j < npath; ++j)
				duDebugDrawNavMeshPoly(&dd, *nav, path[j], duRGBA(255, 255, 255, 24));
		}
	}

	if (m_targetRef)
		duDebugDrawCross(&dd, m_targetPos[0], m_targetPos[1] + 0.1f, m_targetPos[2], rad, duRGBA(255, 255, 255, 192), 2.0f);

	// Occupancy grid.
	if (m_toolParams.m_showGrid)
	{
		float gridy = -FLT_MAX;
		for (int i = 0; i < crowd->getAgentCount(); ++i)
		{
			const dtCrowdAgent* ag = crowd->getAgent(i);
			if (!ag->active) continue;
			const float* pos = ag->corridor.getPos();
			gridy = dtMax(gridy, pos[1]);
		}
		gridy += 1.0f;

		dd.begin(DU_DRAW_QUADS);
		const dtProximityGrid* grid = crowd->getGrid();
		const int* bounds = grid->getBounds();
		const float cs = grid->getCellSize();
		for (int y = bounds[1]; y <= bounds[3]; ++y)
		{
			for (int x = bounds[0]; x <= bounds[2]; ++x)
			{
				const int count = grid->getItemCountAt(x, y);
				if (!count) continue;
				unsigned int col = duRGBA(128, 0, 0, dtMin(count * 40, 255));
				dd.vertex(x*cs, gridy, y*cs, col);
				dd.vertex(x*cs, gridy, y*cs + cs, col);
				dd.vertex(x*cs + cs, gridy, y*cs + cs, col);
				dd.vertex(x*cs + cs, gridy, y*cs, col);
			}
		}
		dd.end();
	}

	// Corners & co
	for (int i = 0; i < crowd->getAgentCount(); i++)
	{
		if (m_toolParams.m_showDetailAll == false && i != m_agentDebug.idx)
			continue;
		const dtCrowdAgent* ag = crowd->getAgent(i);
		if (!ag->active)
			continue;

		const float radius = ag->params.radius;
		const float* pos = ag->npos;

		if (m_toolParams.m_showCorners)
		{
			if (ag->ncorners)
			{
				dd.begin(DU_DRAW_LINES, 2.0f);
				for (int j = 0; j < ag->ncorners; ++j)
				{
					const float* va = j == 0 ? pos : &ag->cornerVerts[(j - 1) * 3];
					const float* vb = &ag->cornerVerts[j * 3];
					dd.vertex(va[0], va[1] + radius, va[2], duRGBA(128, 0, 0, 192));
					dd.vertex(vb[0], vb[1] + radius, vb[2], duRGBA(128, 0, 0, 192));
				}
				if (ag->ncorners && ag->cornerFlags[ag->ncorners - 1] & DT_STRAIGHTPATH_OFFMESH_CONNECTION)
				{
					const float* v = &ag->cornerVerts[(ag->ncorners - 1) * 3];
					dd.vertex(v[0], v[1], v[2], duRGBA(192, 0, 0, 192));
					dd.vertex(v[0], v[1] + radius * 2, v[2], duRGBA(192, 0, 0, 192));
				}

				dd.end();


				if (m_toolParams.m_anticipateTurns)
				{
					/*					float dvel[3], pos[3];
					calcSmoothSteerDirection(ag->pos, ag->cornerVerts, ag->ncorners, dvel);
					pos[0] = ag->pos[0] + dvel[0];
					pos[1] = ag->pos[1] + dvel[1];
					pos[2] = ag->pos[2] + dvel[2];

					const float off = ag->radius+0.1f;
					const float* tgt = &ag->cornerVerts[0];
					const float y = ag->pos[1]+off;

					dd.begin(DU_DRAW_LINES, 2.0f);

					dd.vertex(ag->pos[0],y,ag->pos[2], duRGBA(255,0,0,192));
					dd.vertex(pos[0],y,pos[2], duRGBA(255,0,0,192));

					dd.vertex(pos[0],y,pos[2], duRGBA(255,0,0,192));
					dd.vertex(tgt[0],y,tgt[2], duRGBA(255,0,0,192));

					dd.end();*/
				}
			}
		}

		if (m_toolParams.m_showCollisionSegments)
		{
			const float* center = ag->boundary.getCenter();
			duDebugDrawCross(&dd, center[0], center[1] + radius, center[2], 0.2f, duRGBA(192, 0, 128, 255), 2.0f);
			duDebugDrawCircle(&dd, center[0], center[1] + radius, center[2], ag->params.collisionQueryRange,
				duRGBA(192, 0, 128, 128), 2.0f);

			dd.begin(DU_DRAW_LINES, 3.0f);
			for (int j = 0; j < ag->boundary.getSegmentCount(); ++j)
			{
				const float* s = ag->boundary.getSegment(j);
				unsigned int col = duRGBA(192, 0, 128, 192);
				if (dtTriArea2D(pos, s, s + 3) < 0.0f)
					col = duDarkenCol(col);

				duAppendArrow(&dd, s[0], s[1] + 0.2f, s[2], s[3], s[4] + 0.2f, s[5], 0.0f, 0.3f, col);
			}
			dd.end();
		}

		if (m_toolParams.m_showNeis)
		{
			duDebugDrawCircle(&dd, pos[0], pos[1] + radius, pos[2], ag->params.collisionQueryRange,
				duRGBA(0, 192, 128, 128), 2.0f);

			dd.begin(DU_DRAW_LINES, 2.0f);
			for (int j = 0; j < ag->nneis; ++j)
			{
				// Get 'n'th active agent.
				// TODO: fix this properly.
				const dtCrowdAgent* nei = crowd->getAgent(ag->neis[j].idx);
				if (nei)
				{
					dd.vertex(pos[0], pos[1] + radius, pos[2], duRGBA(0, 192, 128, 128));
					dd.vertex(nei->npos[0], nei->npos[1] + radius, nei->npos[2], duRGBA(0, 192, 128, 128));
				}
			}
			dd.end();
		}

		if (m_toolParams.m_showOpt)
		{
			dd.begin(DU_DRAW_LINES, 2.0f);
			dd.vertex(m_agentDebug.optStart[0], m_agentDebug.optStart[1] + 0.3f, m_agentDebug.optStart[2], duRGBA(0, 128, 0, 192));
			dd.vertex(m_agentDebug.optEnd[0], m_agentDebug.optEnd[1] + 0.3f, m_agentDebug.optEnd[2], duRGBA(0, 128, 0, 192));
			dd.end();
		}
	}

	// Agent cylinders.

	//for (int i = 0; i < crowd->getAgentCount(); ++i)
	//{
	//	const dtCrowdAgent* ag = crowd->getAgent(i);
	//	if (!ag->active) continue;
	//	
	//	const float radius = ag->params.radius;
	//	const float* pos = ag->npos;
	//	
	//	unsigned int col = duRGBA(0,0,0,32);
	//	if (m_agentDebug.idx == i)
	//		col = duRGBA(255,0,0,128);
	//		
	//	duDebugDrawCircle(&dd, pos[0], pos[1], pos[2], radius, col, 2.0f);
	//}

	if (NavSysDebug::ShowAgents)
	{
		for (int i = 0; i < crowd->getAgentCount(); ++i)
		{
			const dtCrowdAgent* ag = crowd->getAgent(i);
			if (!ag->active) continue;

			const float height = ag->params.height;
			const float radius = ag->params.radius;
			const float* pos = ag->npos;

			unsigned int col = duRGBA(220, 220, 220, 128);
			if (ag->targetState == DT_CROWDAGENT_TARGET_REQUESTING || ag->targetState == DT_CROWDAGENT_TARGET_WAITING_FOR_QUEUE)
				col = duLerpCol(col, duRGBA(128, 0, 255, 128), 32);
			else if (ag->targetState == DT_CROWDAGENT_TARGET_WAITING_FOR_PATH)
				col = duLerpCol(col, duRGBA(128, 0, 255, 128), 128);
			else if (ag->targetState == DT_CROWDAGENT_TARGET_FAILED)
				col = duRGBA(255, 32, 16, 128);
			else if (ag->targetState == DT_CROWDAGENT_TARGET_VELOCITY)
				col = duLerpCol(col, duRGBA(64, 255, 0, 128), 128);

			duDebugDrawCylinder(&dd, pos[0] - radius, pos[1] + radius*0.1f, pos[2] - radius,
				pos[0] + radius, pos[1] + height, pos[2] + radius, col);
		}
	}

	if (m_toolParams.m_showVO)
	{
		for (int i = 0; i < crowd->getAgentCount(); i++)
		{
			if (m_toolParams.m_showDetailAll == false && i != m_agentDebug.idx)
				continue;
			const dtCrowdAgent* ag = crowd->getAgent(i);
			if (!ag->active)
				continue;

			// Draw detail about agent sela
			const dtObstacleAvoidanceDebugData* vod = m_agentDebug.vod;

			const float dx = ag->npos[0];
			const float dy = ag->npos[1] + ag->params.height;
			const float dz = ag->npos[2];

			duDebugDrawCircle(&dd, dx, dy, dz, ag->params.maxSpeed, duRGBA(255, 255, 255, 64), 2.0f);

			dd.begin(DU_DRAW_QUADS);
			for (int j = 0; j < vod->getSampleCount(); ++j)
			{
				const float* p = vod->getSampleVelocity(j);
				const float sr = vod->getSampleSize(j);
				const float pen = vod->getSamplePenalty(j);
				const float pen2 = vod->getSamplePreferredSidePenalty(j);
				unsigned int col = duLerpCol(duRGBA(255, 255, 255, 220), duRGBA(128, 96, 0, 220), (int)(pen * 255));
				col = duLerpCol(col, duRGBA(128, 0, 0, 220), (int)(pen2 * 128));
				dd.vertex(dx + p[0] - sr, dy, dz + p[2] - sr, col);
				dd.vertex(dx + p[0] - sr, dy, dz + p[2] + sr, col);
				dd.vertex(dx + p[0] + sr, dy, dz + p[2] + sr, col);
				dd.vertex(dx + p[0] + sr, dy, dz + p[2] - sr, col);
			}
			dd.end();
		}
	}

	if (NavSysDebug::ShowAgentArrows)
	{
		// Velocity stuff.
		for (int i = 0; i < crowd->getAgentCount(); ++i)
		{
			const dtCrowdAgent* ag = crowd->getAgent(i);
			if (!ag->active) continue;

			const float radius = ag->params.radius;
			const float height = ag->params.height;
			const float* pos = ag->npos;
			const float* vel = ag->vel;
			const float* dvel = ag->dvel;

			unsigned int col = duRGBA(220, 220, 220, 192);
			if (ag->targetState == DT_CROWDAGENT_TARGET_REQUESTING || ag->targetState == DT_CROWDAGENT_TARGET_WAITING_FOR_QUEUE)
				col = duLerpCol(col, duRGBA(128, 0, 255, 192), 32);
			else if (ag->targetState == DT_CROWDAGENT_TARGET_WAITING_FOR_PATH)
				col = duLerpCol(col, duRGBA(128, 0, 255, 192), 128);
			else if (ag->targetState == DT_CROWDAGENT_TARGET_FAILED)
				col = duRGBA(255, 32, 16, 192);
			else if (ag->targetState == DT_CROWDAGENT_TARGET_VELOCITY)
				col = duLerpCol(col, duRGBA(64, 255, 0, 192), 128);

			duDebugDrawCircle(&dd, pos[0], pos[1] + height, pos[2], radius, col, 2.0f);
			//++taken from few lines above
			col = duRGBA(0,0,0,32);
			if (m_agentDebug.idx == i)
				col = duRGBA(255,0,0,128);
			duDebugDrawCircle(&dd, pos[0], pos[1], pos[2], radius, col, 2.0f);

			duDebugDrawArrow(&dd, pos[0], pos[1] + height, pos[2],
				pos[0] + dvel[0], pos[1] + height + dvel[1], pos[2] + dvel[2],
				0.0f, 0.4f, duRGBA(0, 192, 255, 192), (m_agentDebug.idx == i) ? 2.0f : 1.0f);

			duDebugDrawArrow(&dd, pos[0], pos[1] + height, pos[2],
				pos[0] + vel[0], pos[1] + height + vel[1], pos[2] + vel[2],
				0.0f, 0.4f, duRGBA(0, 0, 0, 160), 2.0f);
		}
	}
}
