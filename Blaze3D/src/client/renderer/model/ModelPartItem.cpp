#include "ModelPartItem.h"

#include "engine/RenderingInterface.h"

#include "client/renderer/VertexProducer.h"

#include <client/Client.h>

static VertexProducer g_ItemProducer;

ModelPartItem::ModelPartItem(const glm::vec3& scale, const glm::vec3& offsetPosition, const glm::vec3& rotation)
{
    Size = scale;
    Offset = offsetPosition / 16.0f;
    Rotation = rotation;
    Children = {};
    Center = {};
}

void ModelPartItem::Render(VertexProducer& producer, const glm::mat4& parentTransform)
{
    glm::mat4 model = parentTransform;
    model = glm::translate(model, Offset);
    model = glm::translate(model, Center);
    model = glm::rotate(model, glm::radians(Rotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(Rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(Rotation.z), glm::vec3(0, 0, 1));
    model = glm::translate(model, -Center);
    if (Flipped)
    {
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
    }
    model = glm::scale(model, Size);

    LocalTransform = model;

    for (int i = 0; i < Children.size(); i++)
    {
        Children[i]->Render(producer, model);
    }
}
