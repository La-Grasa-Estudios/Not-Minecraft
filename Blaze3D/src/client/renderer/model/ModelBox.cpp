#include "ModelBox.h"
#include "client/renderer/VertexProducer.h"

ModelBox::ModelBox(const glm::vec3& size, const glm::vec3& offsetPosition, TexturedCube quads[6])
{
	Size = size / 16.0f;
	Offset = offsetPosition / 16.0f;
	Rotation = glm::vec3(0.0f);
    Children = {};
    Center = {};
	if (quads)
	{
		for (int i = 0; i < 6; i++)
		{
			Quads[i] = quads[i];
            for (int j = 0; j < 4; j++)
            {
                Quads[i].Uvs[j] /= glm::vec2(64.0f, 32.0f);
            }
		}
	}
	else
	{
		// Default UVs
		for (int i = 0; i < 6; i++)
		{
			Quads[i].Uvs[0] = glm::vec2(0.0f, 0.0f);
			Quads[i].Uvs[1] = glm::vec2(1.0f, 0.0f);
			Quads[i].Uvs[2] = glm::vec2(1.0f, 1.0f);
			Quads[i].Uvs[3] = glm::vec2(0.0f, 1.0f);
		}
	}
}

void ModelBox::Render(VertexProducer& producer, const glm::mat4& parentTransform)
{
    // Build local transform
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

    glm::mat3 normalMatrix = glm::inverse(glm::transpose(model));

    // Half extents
    glm::vec3 he = Size * 0.5f;

    // 8 corners of the box
    glm::vec3 corners[8] = {
        {-he.x, -he.y, -he.z}, // 0
        { he.x, -he.y, -he.z}, // 1
        { he.x,  he.y, -he.z}, // 2
        {-he.x,  he.y, -he.z}, // 3
        {-he.x, -he.y,  he.z}, // 4
        { he.x, -he.y,  he.z}, // 5
        { he.x,  he.y,  he.z}, // 6
        {-he.x,  he.y,  he.z}  // 7
    };

    glm::vec3 faceNormals[6] = {
    { 0,  0, -1}, // back
    { 0,  0,  1}, // front
    {-1,  0,  0}, // left
    { 1,  0,  0}, // right
    { 0,  1,  0}, // top
    { 0, -1,  0}  // bottom
    };

    // Faces defined by 4 corner indices
    int faces[6][4] = {
        {0,1,2,3}, // back
        {4,5,6,7}, // front
        {0,4,7,3}, // left
        {1,5,6,2}, // right
        {3,2,6,7}, // top
        {0,1,5,4}  // bottom
    };

    // Emit vertices
    for (int f = 0; f < 6; ++f)
    {
        // Transform the face normal
        glm::vec3 n = glm::normalize(normalMatrix * faceNormals[f]);
        producer.SetNormal(n.x, n.y, n.z);

        for (int v = 0; v < 4; ++v)
        {
            glm::vec4 pos = model * glm::vec4(corners[faces[f][v]], 1.0f);
            glm::vec2 uv = Quads[f].Uvs[v];
            producer.AddVertex(pos.x, pos.y, pos.z, uv.x, uv.y);
        }
    }


    for (int i = 0; i < Children.size(); i++)
    {
        Children[i]->Render(producer, model);
    }
}
