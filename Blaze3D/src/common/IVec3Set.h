#pragma once

#include "thirdparty/glm/ext.hpp"
#include <unordered_set>

struct IVec3Hasher
{
	size_t operator()(const glm::ivec3& node) const;
};

struct IVec3Equal {
	bool operator()(const glm::ivec3& a, const glm::ivec3& b) const {
		return a.x == b.x && a.y == b.y && a.z == b.z;
	}
};

using IVec3Set = std::unordered_set<glm::ivec3, IVec3Hasher, IVec3Equal>;