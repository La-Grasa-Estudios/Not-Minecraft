#pragma once

#include "ModelPart.h"

struct TexturedCube
{
	glm::vec2 Uvs[4];
};

class ModelBox : public ModelPart
{
public:
	ModelBox() = default;
	ModelBox(const glm::vec3& size, const glm::vec3& offsetPosition, TexturedCube quads[6]);

	void Render(VertexProducer& producer, const glm::mat4& parentTransform) override;

	TexturedCube Quads[6];
};