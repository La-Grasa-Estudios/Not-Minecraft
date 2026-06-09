#pragma once

class VertexProducer;
#include "thirdparty/glm/ext.hpp"
#include <vector>

class ModelPart
{
public:
	ModelPart() = default;
	glm::vec3 Rotation, Offset, Size, Center, Color = glm::vec4(1.0f);
	bool Flipped = false;
	virtual void Render(VertexProducer& producer, const glm::mat4& parentTransform);
	void AddChild(ModelPart* part);

	std::vector<ModelPart*> Children;
};