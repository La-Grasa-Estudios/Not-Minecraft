#pragma once

#include "thirdparty/glm/ext.hpp"
#include <vector>

struct NavPath
{
	std::vector<glm::ivec3> Nodes;
	glm::ivec3 Target;

	bool Completed = false;

	int NavIndex = 0;

	glm::ivec3 GetCurrentNode() const;
	void IncrementIndex();
	bool IsComplete() const;
};