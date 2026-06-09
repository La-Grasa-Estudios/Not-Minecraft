#pragma once

#include <thirdparty/glm/ext.hpp>

struct RayCastHit
{
	glm::vec3 HitPosition;
	glm::ivec3 HitBlock;
	int Face;
};