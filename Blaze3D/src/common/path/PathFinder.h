#pragma once

#include <unordered_map>
#include <queue>

#include "NavPath.h"
#include "common/IVec3Set.h"

class World;

class PathFinder
{
public:
	PathFinder(World* world);
	~PathFinder();

	NavPath FindPath(const glm::ivec3& start, const glm::ivec3& goal);

private:

	bool IsPointWalkable(const glm::ivec3& point, const glm::ivec3& start, const glm::ivec3& goal, const glm::ivec3& parent);
	bool IsPointSafe(const glm::ivec3& point);

	struct PathNode
	{
		glm::ivec3 pos;
		float f, g, h;
		glm::ivec3 parent;
	};



	struct PathNodeComparator
	{
		bool operator()(const PathNode a, const PathNode b) const;
	};

	std::unordered_map<glm::ivec3, PathNode, IVec3Hasher, IVec3Equal> m_ClosedSet;
	std::unordered_set<glm::ivec3, IVec3Hasher, IVec3Equal> m_OpenSet;
	std::priority_queue<PathNode, std::vector<PathNode>, PathNodeComparator> m_OpenQueue;

	World* m_World;
};