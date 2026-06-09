#pragma once

#include <thirdparty/glm/ext.hpp>

class ColorizerGrass
{
public:
	static glm::vec3 GetColor(float temperature, float humidity);
};