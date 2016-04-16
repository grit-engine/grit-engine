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

#ifndef CROWDMANAGER_H
#define CROWDMANAGER_H

#include "navigation_manager.h"
#include "DetourNavMesh.h"
#include "DetourObstacleAvoidance.h"
#include "DetourCrowd.h"

struct CrowdToolParams
{
	bool m_expandSelectedDebugDraw;
	bool m_showCorners;
	bool m_showCollisionSegments;
	bool m_showPath;
	bool m_showVO;
	bool m_showOpt;
	bool m_showNeis;
	
	bool m_expandDebugDraw;
	bool m_showLabels;
	bool m_showGrid;
	bool m_showNodes;
	bool m_showPerfGraph;
	bool m_showDetailAll;
	
	bool m_expandOptions;
	bool m_anticipateTurns;
	bool m_optimizeVis;
	bool m_optimizeTopo;
	bool m_obstacleAvoidance;
	float m_obstacleAvoidanceType;
	bool m_separation;
	float m_separationWeight;
};

class CrowdTool
{
	NavigationManager* m_nvmgr;
	
	enum ToolMode
	{
		TOOLMODE_CREATE,
		TOOLMODE_MOVE_TARGET,
		TOOLMODE_SELECT,
		TOOLMODE_TOGGLE_POLYS,
	};
	ToolMode m_mode;

	dtNavMesh* m_nav;
	dtCrowd* m_crowd;
	float m_targetPos[3];
	dtPolyRef m_targetRef;
	dtCrowdAgentDebugInfo m_agentDebug;
	dtObstacleAvoidanceDebugData* m_vod;
	static const int MAX_AGENTS = 128;

	CrowdToolParams m_toolParams;

public:
	CrowdTool();
	virtual ~CrowdTool();

	virtual void init(NavigationManager* nvmgr);
	virtual void reset();
	virtual void step();
	virtual void update(const float dt);
	virtual void render();

	int addAgent(const float* pos);
	void removeAgent(const int idx);
	void hilightAgent(const int idx);
	void updateAgentParams();
	int hitTestAgents(const float* s, const float* p);
	void setMoveTarget(const float* p, bool adjust);
	void updateTick(const float dt);
	inline CrowdToolParams* getToolParams() { return &m_toolParams; }
};

#endif // CROWDMANAGER_H
