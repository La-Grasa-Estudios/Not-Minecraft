#pragma once

#include "ModelPart.h"

class ModelPartItem : public ModelPart
{
public:
	ModelPartItem() = default;
	ModelPartItem(const glm::vec3& scale, const glm::vec3& offsetPosition, const glm::vec3& rotation);

	void Render(VertexProducer& producer, const glm::mat4& parentTransform) override;

	glm::mat4 LocalTransform;
};