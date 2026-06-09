#include "ModelPig.h"

#include "client/Client.h"
#include "engine/RenderingInterface.h"

#include "engine/System.h"

ModelPig::ModelPig()
{
	// Side: (0, 0) (0, 1) (1, 1) (1, 0)

	TexturedCube bodyQuads[6] = {
{ { { 46.f, 16.f }, { 56.f, 16.f }, { 56.f, 8.f }, { 46.f, 8.f } } }, // Back
{ { { 36.f, 16.f }, { 46.f, 16.f }, { 46.f, 8.f }, { 36.f, 8.f } } }, // Front
{ { { 28.f, 16.f }, { 28.f, 32.f }, { 36.f, 32.f }, { 36.f, 16.f } } }, // Left
{ { { 56.f, 16.f }, { 56.f, 32.f }, { 64.f, 32.f }, { 64.f, 16.f } } }, // Right
{ { { 46.f, 32.f }, { 56.f, 32.f }, { 56.f, 16.f }, { 46.f, 16.f } } }, // Top
{ { { 36.f, 32.f }, { 46.f, 32.f }, { 46.f, 16.f }, { 36.f, 16.f } } }, // Bottom
	};

	TexturedCube legQuads[6] = {
	{ { { 8.f, 26.f }, { 12.f, 26.f }, { 12.f, 20.f }, { 8.f, 20.f } } }, // Back
	{ { { 12.f, 26.f }, { 16.f, 26.f }, { 16.f, 20.f }, { 12.f, 20.f } } }, // Front
	{ { { 8.f, 26.f }, { 4.f, 26.f }, { 4.f, 20.f }, { 8.f, 20.f } } }, // Left
	{ { { 0.f, 26.f }, { 4.f, 26.f }, { 4.f, 20.f }, { 0.f, 20.f } } }, // Right
	{ { { 8.f, 20.f }, { 4.f, 20.f }, { 4.f, 16.f }, { 8.f, 16.f } } }, // Top
	{ { { 8.f, 16.f }, { 12.f, 16.f }, { 12.f, 20.f }, { 8.f, 20.f } } }, // Bottom
	};

	TexturedCube headQuads[6] = {
	{ { { 24.f, 16.f }, { 32.f, 16.f }, { 32.f, 8.f }, { 24.f, 8.f } } }, // Back
	{ { { 8.f, 16.f }, { 16.f, 16.f }, { 16.f, 8.f }, { 8.f, 8.f } } }, // Front
	{ { { 0.f, 16.f }, { 8.f, 16.f }, { 8.f, 8.f }, { 0.f, 8.f } } }, // Left
	{ { { 24.f, 16.f }, { 16.f, 16.f }, { 16.f, 8.f }, { 24.f, 8.f } } }, // Right
	{ { { 8.f, 0.f }, { 16.f, 0.f }, { 16.f, 8.f }, { 8.f, 8.f } } }, // Top
	{ { { 16.f, 0.f }, { 24.f, 0.f }, { 24.f, 8.f }, { 16.f, 8.f } } }, // Bottom
	};

	Body = ModelBox(glm::vec3(10.0f, 8.0f, 16.0f), glm::vec3(0.0f, 0.0f, 0.0f), bodyQuads);

	LegLeft = ModelBox(glm::vec3(4.0f, 6.0f, 4.0f), glm::vec3(-3.0f, -7.0f, -6.0f), legQuads);
	LegRight = ModelBox(glm::vec3(4.0f, 6.0f, 4.0f), glm::vec3(3.0f, -7.0f, -6.0f), legQuads);

	ArmLeft = ModelBox(glm::vec3(4.0f, 6.0f, 4.0f), glm::vec3(-3.0f, -7.0f, 6.0f), legQuads);
	ArmRight = ModelBox(glm::vec3(4.0f, 6.0f, 4.0f), glm::vec3(3.0f, -7.0f, 6.0f), legQuads);

	Head = ModelBox(glm::vec3(8.0f, 8.0f, 8.0f), glm::vec3(0.0f, 2.0f, 10.0f), headQuads);

	Body.AddChild(&LegLeft);
	Body.AddChild(&LegRight);
	Body.AddChild(&ArmLeft);
	Body.AddChild(&ArmRight);
	Body.AddChild(&Head);

	ArmLeft.Center.y = 3.0f / 16.0f;
	ArmRight.Center.y = 3.0f / 16.0f;
	LegLeft.Center.y = 3.0f / 16.0f;
	LegRight.Center.y = 3.0f / 16.0f;
	Head.Center.z = -3.0f / 16.0f;

	Texture = Client::GetInstance().GraphicsContext->LoadTexture("mc/textures/mob/pig.png");
	Client::GetInstance().GraphicsContext->SetTextureFilter(Texture, RI_FILTER_NEAREST);
}

void ModelPig::Render(VertexProducer& producer, float walkingProgress, float digProgress, float idleProgress, float armsAngle, float legSwingFactor, bool applyWalkToArms)
{
	const float walkAngle = 35.0f;

	Body.Offset.y = -4.0f / 16.0f;

	ArmLeft.Rotation.x = glm::sin(walkingProgress) * walkAngle * legSwingFactor;
	ArmRight.Rotation.x = -glm::sin(walkingProgress) * walkAngle * legSwingFactor;

	LegLeft.Rotation.x = -glm::sin(walkingProgress) * walkAngle * legSwingFactor;
	LegRight.Rotation.x = glm::sin(walkingProgress) * walkAngle * legSwingFactor;

	Body.Render(producer, glm::mat4(1.0f));
}
