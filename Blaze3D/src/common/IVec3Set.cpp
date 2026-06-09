#include "IVec3Set.h"

size_t IVec3Hasher::operator()(const glm::ivec3& node) const
{
	uint64_t x = static_cast<uint64_t>(node.x) & 0x1FFFFF;
	uint64_t y = static_cast<uint64_t>(node.y) & 0x1FFFFF;
	uint64_t z = static_cast<uint64_t>(node.z) & 0x1FFFFF;

	uint64_t hash = (x << 0) | (y << 21) | (z << 42);
	return static_cast<size_t>(hash);
}
