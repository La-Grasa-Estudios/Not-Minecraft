#include "PathFinder.h"

#include "common/World.h"

#include <functional>
#include <algorithm>

bool PathFinder::PathNodeComparator::operator()(const PathNode a, const PathNode b) const
{
	return a.f > b.f;
}

glm::ivec3 NavPath::GetCurrentNode() const
{
	return Nodes[NavIndex];
}

void NavPath::IncrementIndex()
{
	NavIndex++;
	if (NavIndex >= Nodes.size())
	{
		Completed = true;
	}
}

bool NavPath::IsComplete() const
{
	return Completed;
}

PathFinder::PathFinder(World* world)
{
	m_World = world;
}

PathFinder::~PathFinder()
{
}

NavPath PathFinder::FindPath(const glm::ivec3& start, const glm::ivec3& goal)
{
	m_ClosedSet.clear();
	m_OpenSet.clear();
	m_OpenQueue = std::priority_queue<PathNode, std::vector<PathNode>, PathNodeComparator>();

	{
		float hCost = static_cast<float>(glm::length(glm::vec3(start - goal)));
		m_OpenQueue.push(PathNode{ start, hCost, 0.0f, hCost });
		m_OpenSet.insert(m_OpenQueue.top().pos);
	}

	static const glm::ivec3 neighbourOffsets[] =
	{
		{ 0 , -1, 0  },
		{ 0 ,  1, 0  },

		{ -1, 0 , 0  },
		{  1, 0 , 0  },
		{  0, 0 ,-1 },
		{  0, 0 , 1  },

		{ -1, 0 , 1  },
		{  1, 0 , -1  },
		{  -1, 0 ,-1 },
		{  1, 0 , 1  },
	};

	bool found = false;
	bool nearGoal = false;
	int iterationsCloseToGoal = 0;

	while (!m_OpenQueue.empty())
	{
		auto currentNode = m_OpenQueue.top();
		m_OpenQueue.pop();
		m_OpenSet.erase(currentNode.pos);

		m_ClosedSet[currentNode.pos] = currentNode;

		if (glm::distance((glm::vec3)currentNode.pos, (glm::vec3)goal) < 2.0f)
		{
			nearGoal = true;
		}

		if (nearGoal)
		{
			iterationsCloseToGoal++;
			if (iterationsCloseToGoal > 100)
			{
				break;
			}
		}

		if (currentNode.pos == goal)
		{
			found = true;
			break;
		}

		for (int i = 0; i < 10; i++)
		{
			glm::ivec3 neighbourPos = currentNode.pos + neighbourOffsets[i];
			auto neighbourPoint = m_ClosedSet.find(neighbourPos);

			if (!IsPointWalkable(neighbourPos, start, goal, currentNode.pos))
			{
				continue;
			}

			float distanceToStart = static_cast<float>(glm::length(glm::vec3(neighbourPos - start)));
			float gCost = distanceToStart + currentNode.g;
			float hCost = static_cast<float>(glm::length(glm::vec3(neighbourPos - goal)));
			float fCost = gCost + hCost;

			if (distanceToStart > 32.0f)
			{
				break;
			}

			if (m_OpenSet.find(neighbourPos) == m_OpenSet.end() && neighbourPoint == m_ClosedSet.end())
			{
				auto node = PathNode{ neighbourPos, fCost, gCost, hCost, currentNode.pos };
				m_OpenQueue.push(node);
				m_OpenSet.insert(neighbourPos);
			}

			if (neighbourPoint != m_ClosedSet.end())
			{
				auto& n = neighbourPoint->second;
				if (n.pos != start && gCost < n.g)
				{
					n.f = fCost;
					n.g = gCost;
					n.h = hCost;
					n.parent = currentNode.pos;
				}
			}
		}
	}

	NavPath path;

	if (found)
	{
		auto currentNode = m_ClosedSet[goal];
		while (currentNode.pos != start)
		{
			path.Nodes.push_back(currentNode.pos);
			currentNode = m_ClosedSet[currentNode.parent];
		}

		path.Nodes.push_back(start);
		std::reverse(path.Nodes.begin(), path.Nodes.end());
	}
	else {
		auto bestIt = std::min_element(
			m_ClosedSet.begin(), m_ClosedSet.end(),
			[](auto& a, auto& b) { return a.second.h < b.second.h; }
		);

		if (bestIt != m_ClosedSet.end()) {
			auto currentNode = bestIt->second;
			while (currentNode.pos != start) {
				path.Nodes.push_back(currentNode.pos);
				currentNode = m_ClosedSet[currentNode.parent];
			}
			path.Nodes.push_back(start);
			std::reverse(path.Nodes.begin(), path.Nodes.end());
		}
	}

	path.Target = goal;

	return path;
}

bool PathFinder::IsPointWalkable(const glm::ivec3& point, const glm::ivec3& start, const glm::ivec3& goal, const glm::ivec3& parent)
{
	// If the block itself is solid, not walkable
	if (m_World->GetBlock(point.x, point.y, point.z).GetDef()->IsSolid()
		|| m_World->GetBlock(point.x, point.y + 1, point.z).GetDef()->IsSolid()
		|| m_World->GetBlock(point.x, point.y - 1, point.z).GetDef()->GetId() == BLOCK_WATER)
		return false;

	static const glm::ivec3 horizontalOffsets[] =
	{
		{ -1, 0, 0 },
		{  1, 0, 0 },
		{  0, 0,-1 },
		{  0, 0, 1 }
	};

	glm::ivec3 below(point.x, point.y - 1, point.z);

	if (m_World->GetBlock(below.x, below.y, below.z).GetDef()->IsSolid())
	{
		return true;
	}

	if (parent != glm::ivec3(0))
	{
		if (IsPointSafe(parent))
		{
			if (point.y == parent.y + 1)
			{
				for (auto offset : horizontalOffsets)
				{
					glm::ivec3 checkPos = point + offset;
					if (IsPointSafe(checkPos))
					{
						return true;
					}
				}
			}
			if (point.y == parent.y && (point.x == parent.x || point.z == parent.z))
			{
				for (int dy = -1; dy >= -3; dy--)
				{
					if (IsPointSafe(point + glm::ivec3(0, dy, 0)))
					{
						return true;
					}
				}
			}
		}
		if (point.y == parent.y - 1 && (point.x == parent.x || point.z == parent.z))
		{
			for (int dy = -1; dy >= -3; dy--)
			{
				if (IsPointSafe(point + glm::ivec3(0, dy, 0)))
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool PathFinder::IsPointSafe(const glm::ivec3& point)
{
	return m_World->GetBlock(point.x, point.y - 1, point.z).GetDef()->IsSolid() && !m_World->GetBlock(point.x, point.y, point.z).GetDef()->IsSolid();
}

