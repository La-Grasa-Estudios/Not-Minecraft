#include "ModelPart.h"

void ModelPart::Render(VertexProducer& producer, const glm::mat4& parentTransform)
{
}

void ModelPart::AddChild(ModelPart* part)
{
	Children.push_back(part);
}
