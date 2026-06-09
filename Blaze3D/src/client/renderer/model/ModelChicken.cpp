#include "ModelChicken.h"

#include "client/Client.h"
#include "engine/RenderingInterface.h"

ModelChicken::ModelChicken()
{
	TexturedCube bodyQuads[6] = {
	{ { { 12.f, 15.f }, { 18.f, 15.f }, { 18.f, 9.f }, { 12.f, 9.f } } }, // Back
	{ { { 6.f, 15.f }, { 12.f, 15.f }, { 12.f, 9.f }, { 6.f, 9.f } } }, // Front
	{ { { 0.f, 15.f }, { 0.f, 23.f }, { 6.f, 23.f }, { 6.f, 15.f } } }, // Left
	{ { { 0.f, 15.f }, { 0.f, 23.f }, { 6.f, 23.f }, { 6.f, 15.f } } }, // Right
	{ { { 0.f, 23.f }, { 6.f, 23.f }, { 6.f, 15.f }, { 0.f, 15.f } } }, // Top
	{ { { 0.f, 23.f }, { 6.f, 23.f }, { 6.f, 15.f }, { 0.f, 15.f } } }, // Bottom
	};

	TexturedCube wingQuads[6] = {
	{ { { 12.f, 13.f }, { 13.f, 13.f }, { 13.f, 9.f }, { 12.f, 9.f } } }, // Back
	{ { { 6.f, 13.f }, { 7.f, 13.f }, { 7.f, 9.f }, { 6.f, 9.f } } }, // Front
	{ { { 0.f, 15.f }, { 0.f, 21.f }, { 4.f, 21.f }, { 4.f, 15.f } } }, // Left
	{ { { 0.f, 15.f }, { 0.f, 21.f }, { 4.f, 21.f }, { 4.f, 15.f } } }, // Right
	{ { { 1.f, 15.f }, { 0.f, 15.f }, { 0.f, 21.f }, { 1.f, 21.f } } }, // Top
	{ { { 1.f, 15.f }, { 0.f, 15.f }, { 0.f, 21.f }, { 1.f, 21.f } } }, // Bottom
	};

	TexturedCube headQuads[6] = {
	{ { { 7.f, 9.f }, { 11.f, 9.f }, { 11.f, 3.f }, { 7.f, 3.f } } }, // Back
	{ { { 3.f, 9.f }, { 7.f, 9.f }, { 7.f, 3.f }, { 3.f, 3.f } } }, // Front
	{ { { 0.f, 9.f }, { 3.f, 9.f }, { 3.f, 3.f }, { 0.f, 3.f } } }, // Left
	{ { { 0.f, 9.f }, { 3.f, 9.f }, { 3.f, 3.f }, { 0.f, 3.f } } }, // Right
	{ { { 3.f, 3.f }, { 7.f, 3.f }, { 7.f, 0.f }, { 3.f, 0.f } } }, // Top
	{ { { 3.f, 3.f }, { 7.f, 3.f }, { 7.f, 0.f }, { 3.f, 0.f } } }, // Bottom
	};

	TexturedCube wattleQuads[6] = {
	{ { { 16.f, 8.f }, { 20.f, 8.f }, { 20.f, 4.f }, { 16.f, 4.f } } }, // Back
	{ { { 16.f, 8.f }, { 20.f, 8.f }, { 20.f, 4.f }, { 16.f, 4.f } } }, // Front
	{ { { 16.f, 8.f }, { 20.f, 8.f }, { 20.f, 4.f }, { 16.f, 4.f } } }, // Left
	{ { { 16.f, 8.f }, { 20.f, 8.f }, { 20.f, 4.f }, { 16.f, 4.f } } }, // Right
	{ { { 16.f, 8.f }, { 20.f, 8.f }, { 20.f, 4.f }, { 16.f, 4.f } } }, // Top
	{ { { 16.f, 8.f }, { 20.f, 8.f }, { 20.f, 4.f }, { 16.f, 4.f } } }, // Bottom
	};

	TexturedCube beakQuads[6] = {
	{ { { 16.f, 8.f }, { 20.f, 8.f }, { 20.f, 4.f }, { 16.f, 4.f } } }, // Back
	{ { { 16.f, 4.f }, { 20.f, 4.f }, { 20.f, 2.f }, { 16.f, 2.f } } }, // Front
	{ { { 14.f, 4.f }, { 16.f, 4.f }, { 16.f, 2.f }, { 14.f, 2.f } } }, // Left
	{ { { 20.f, 4.f }, { 22.f, 4.f }, { 22.f, 2.f }, { 20.f, 2.f } } }, // Right
	{ { { 20.f, 0.f }, { 16.f, 0.f }, { 16.f, 2.f }, { 20.f, 2.f } } }, // Top
	{ { { 20.f, 0.f }, { 24.f, 0.f }, { 24.f, 2.f }, { 20.f, 2.f } } }, // Bottom
	};

	TexturedCube legQuads[6] = {
	{ { { 36.f, 8.f }, { 37.f, 8.f }, { 37.f, 3.f }, { 36.f, 3.f } } }, // Back
	{ { { 36.f, 8.f }, { 37.f, 8.f }, { 37.f, 3.f }, { 36.f, 3.f } } }, // Front
	{ { { 36.f, 8.f }, { 37.f, 8.f }, { 37.f, 3.f }, { 36.f, 3.f } } }, // Left
	{ { { 36.f, 8.f }, { 37.f, 8.f }, { 37.f, 3.f }, { 36.f, 3.f } } }, // Right
	{ { { 36.f, 8.f }, { 37.f, 8.f }, { 37.f, 3.f }, { 36.f, 3.f } } }, // Top
	{ { { 36.f, 8.f }, { 37.f, 8.f }, { 37.f, 3.f }, { 36.f, 3.f } } }, // Bottom
	};

	TexturedCube clawQuads[6] = {
	{ { { 35.f, 0.f }, { 32.f, 0.f }, { 32.f, 3.f }, { 35.f, 3.f } } }, // Back
	{ { { 35.f, 0.f }, { 32.f, 0.f }, { 32.f, 3.f }, { 35.f, 3.f } } }, // Front
	{ { { 35.f, 0.f }, { 32.f, 0.f }, { 32.f, 3.f }, { 35.f, 3.f } } }, // Left
	{ { { 35.f, 0.f }, { 32.f, 0.f }, { 32.f, 3.f }, { 35.f, 3.f } } }, // Right
	{ { { 35.f, 0.f }, { 32.f, 0.f }, { 32.f, 3.f }, { 35.f, 3.f } } }, // Top
	{ { { 35.f, 0.f }, { 32.f, 0.f }, { 32.f, 3.f }, { 35.f, 3.f } } }, // Bottom
	};

	Body = ModelBox(glm::vec3(6.0f, 6.0f, 8.0f), glm::vec3(0.0f, 0.0f, 0.0f), bodyQuads);
	Body.Offset.y = -4.0f / 16.0f;

	WingLeft = ModelBox(glm::vec3(1.0f, 4.0f, 6.0f), glm::vec3(3.5f, 1.0f, 0.0f), wingQuads);
	WingRight = ModelBox(glm::vec3(1.0f, 4.0f, 6.0f), glm::vec3(-3.5f, 1.0f, 0.0f), wingQuads);

	Head = ModelBox(glm::vec3(4.0f, 6.0f, 3.0f), glm::vec3(0.0f, 4.0f, 4.5f), headQuads);
	Wattle = ModelBox(glm::vec3(2.0f, 2.0f, 1.0f), glm::vec3(0.0f, -2.0f, 2.0f), wattleQuads);
	Beak = ModelBox(glm::vec3(4.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f), beakQuads);

	LegLeft = ModelBox(glm::vec3(1.0f, 5.0f, 0.0f), glm::vec3(1.5f, -5.5f, 0.0f), legQuads);
	LegRight = ModelBox(glm::vec3(1.0f, 5.0f, 0.0f), glm::vec3(-1.5f, -5.5f, 0.0f), legQuads);

	ClawRight = ModelBox(glm::vec3(3.0f, 0.0f, 3.0f), glm::vec3(0.0f, -2.5f, 1.5f), clawQuads);
	ClawLeft = ModelBox(glm::vec3(3.0f, 0.0f, 3.0f), glm::vec3(0.0f, -2.5f, 1.5f), clawQuads);

	LegLeft.AddChild(&ClawLeft);
	LegRight.AddChild(&ClawRight);

	LegLeft.Center.y = 2.5f / 16.0f;
	LegRight.Center.y = 2.5f / 16.0f;

	WingLeft.Center.y = 2.0f / 16.0f;
	WingRight.Center.y = 2.0f / 16.0f;

	WingLeft.Center.x = -0.5f / 16.0f;
	WingRight.Center.x = 0.5f / 16.0f;

	Body.AddChild(&WingLeft);
	Body.AddChild(&WingRight);

	Body.AddChild(&LegLeft);
	Body.AddChild(&LegRight);

	Body.AddChild(&Head);
	Head.AddChild(&Wattle);
	Head.AddChild(&Beak);

	Texture = Client::GetInstance().GraphicsContext->LoadTexture("mc/textures/mob/chicken.png");
	Client::GetInstance().GraphicsContext->SetTextureFilter(Texture, RI_FILTER_NEAREST);
}

void ModelChicken::Render(VertexProducer& producer, float walkingProgress, float digProgress, float idleProgress, float armsAngle, float legSwingFactor, bool applyWalkToArms)
{
	const float walkAngle = 45.0f;

	LegLeft.Rotation.x = -glm::sin(walkingProgress) * walkAngle * legSwingFactor;
	LegRight.Rotation.x = glm::sin(walkingProgress) * walkAngle * legSwingFactor;

	WingLeft.Rotation.z = (-glm::cos(Parameters[0]) + 1.0f) * 45.0f;
	WingRight.Rotation.z = -(-glm::cos(Parameters[0]) + 1.0f) * 45.0f;

	Body.Render(producer, glm::mat4(1.0f));

}
