#include "ModelCow.h"

#include "client/Client.h"
#include "engine/RenderingInterface.h"

ModelCow::ModelCow()
{
	// Side: (0, 0) (0, 1) (1, 1) (1, 0)

	TexturedCube bodyQuads[6] = {
	{ { { 40.f, 14.f }, { 52.f, 14.f }, { 52.f, 4.f }, { 40.f, 4.f } } }, // Back
	{ { { 28.f, 14.f }, { 40.f, 14.f }, { 40.f, 4.f }, { 28.f, 4.f } } }, // Front
	{ { { 18.f, 14.f }, { 18.f, 32.f }, { 28.f, 32.f }, { 28.f, 14.f } } }, // Left
	{ { { 38.f, 14.f }, { 38.f, 32.f }, { 48.f, 32.f }, { 48.f, 14.f } } }, // Right
	{ { { 38.f, 32.f }, { 48.f, 32.f }, { 48.f, 14.f }, { 38.f, 14.f } } }, // Top
	{ { { 28.f, 32.f }, { 38.f, 32.f }, { 38.f, 14.f }, { 28.f, 14.f } } }, // Bottom
	};

	TexturedCube legQuads[6] = {
	{ { { 8.f, 32.f }, { 12.f, 32.f }, { 12.f, 20.f }, { 8.f, 20.f } } }, // Back
	{ { { 12.f, 32.f }, { 16.f, 32.f }, { 16.f, 20.f }, { 12.f, 20.f } } }, // Front
	{ { { 8.f, 32.f }, { 4.f, 32.f }, { 4.f, 20.f }, { 8.f, 20.f } } }, // Left
	{ { { 0.f, 32.f }, { 4.f, 32.f }, { 4.f, 20.f }, { 0.f, 20.f } } }, // Right
	{ { { 8.f, 20.f }, { 4.f, 20.f }, { 4.f, 16.f }, { 8.f, 16.f } } }, // Top
	{ { { 8.f, 16.f }, { 12.f, 16.f }, { 12.f, 20.f }, { 8.f, 20.f } } }, // Bottom
	};

	TexturedCube headQuads[6] = {
	{ { { 20.f, 14.f }, { 26.f, 14.f }, { 26.f, 6.f }, { 20.f, 6.f } } }, // Back
	{ { { 6.f, 14.f }, { 14.f, 14.f }, { 14.f, 6.f }, { 6.f, 6.f } } }, // Front
	{ { { 6.f, 14.f }, { 0.f, 14.f }, { 0.f, 6.f }, { 6.f, 6.f } } }, // Left
	{ { { 20.f, 14.f }, { 14.f, 14.f }, { 14.f, 6.f }, { 20.f, 6.f } } }, // Right
	{ { { 6.f, 0.f }, { 14.f, 0.f }, { 14.f, 6.f }, { 6.f, 6.f } } }, // Top
	{ { { 14.f, 0.f }, { 22.f, 0.f }, { 22.f, 6.f }, { 14.f, 6.f } } }, // Bottom
	};

	Body = ModelBox(glm::vec3(10.0f, 10.0f, 18.0f), glm::vec3(0.0f, 0.0f, 0.0f), bodyQuads);

	LegLeft = ModelBox(glm::vec3(4.0f, 12.0f, 4.0f), glm::vec3(-3.0f, -11.0f, -7.0f), legQuads);
	LegRight = ModelBox(glm::vec3(4.0f, 12.0f, 4.0f), glm::vec3(3.0f, -11.0f, -7.0f), legQuads);

	ArmLeft = ModelBox(glm::vec3(4.0f, 12.0f, 4.0f), glm::vec3(-3.0f, -11.0f, 7.0f), legQuads);
	ArmRight = ModelBox(glm::vec3(4.0f, 12.0f, 4.0f), glm::vec3(3.0f, -11.0f, 7.0f), legQuads);

	Head = ModelBox(glm::vec3(8.0f, 8.0f, 6.0f), glm::vec3(0.0f, 3.0f, 12.0f), headQuads);

	Body.AddChild(&LegLeft);
	Body.AddChild(&LegRight);
	Body.AddChild(&ArmLeft);
	Body.AddChild(&ArmRight);
	Body.AddChild(&Head);

	ArmLeft.Center.y = 5.5f / 16.0f;
	ArmRight.Center.y = 5.5f / 16.0f;
	LegLeft.Center.y = 5.5f / 16.0f;
	LegRight.Center.y = 5.5f / 16.0f;
	Head.Center.z = -3.0f / 16.0f;

	Texture = Client::GetInstance().GraphicsContext->LoadTexture("mc/textures/mob/cow.png");
	Client::GetInstance().GraphicsContext->SetTextureFilter(Texture, RI_FILTER_NEAREST);
}

void ModelCow::Render(VertexProducer& producer, float walkingProgress, float digProgress, float idleProgress, float armsAngle, float legSwingFactor, bool applyWalkToArms)
{
	const float walkAngle = 35.0f;

	Body.Offset.y = -4.0f / 16.0f;

	ArmLeft.Rotation.x = glm::sin(walkingProgress) * walkAngle * legSwingFactor;
	ArmRight.Rotation.x = -glm::sin(walkingProgress) * walkAngle * legSwingFactor;

	LegLeft.Rotation.x = -glm::sin(walkingProgress) * walkAngle * legSwingFactor;
	LegRight.Rotation.x = glm::sin(walkingProgress) * walkAngle * legSwingFactor;

	Body.Render(producer, glm::mat4(1.0f));
}
