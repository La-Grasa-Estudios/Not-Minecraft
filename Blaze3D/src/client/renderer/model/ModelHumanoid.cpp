#include "ModelHumanoid.h"

#include "client/ResourceManager.h"
#include "client/Client.h"

ModelHumanoid::ModelHumanoid()
{
	TexturedCube torsoQuads[6] = {
		{ { { 32.f, 32.f }, { 40.f, 32.f }, { 40.f, 20.f }, { 32.f, 20.f } } }, // Back
		{ { { 20.f, 32.f }, { 28.f, 32.f }, { 28.f, 20.f }, { 20.f, 20.f } } }, // Front
		{ { { 16.f, 32.f }, { 20.f, 32.f }, { 20.f, 20.f }, { 16.f, 20.f } } }, // Left
		{ { { 32.f, 32.f }, { 28.f, 32.f }, { 28.f, 20.f }, { 32.f, 20.f } } }, // Right
		{ { { 20.f, 20.f }, { 28.f, 20.f }, { 28.f, 16.f }, { 20.f, 16.f } } }, // Top
		{ { { 28.f, 32.f }, { 32.f, 32.f }, { 32.f, 20.f }, { 28.f, 20.f } } }, // Bottom
	};

	TexturedCube armQuads[6] = {
		{ { { 48.f, 32.f }, { 44.f, 32.f }, { 44.f, 20.f }, { 48.f, 20.f } } }, // Back
		{ { { 48.f, 32.f }, { 44.f, 32.f }, { 44.f, 20.f }, { 48.f, 20.f } } }, // Front
		{ { { 44.f, 32.f }, { 40.f, 32.f }, { 40.f, 20.f }, { 44.f, 20.f } } }, // Left
		{ { { 52.f, 32.f }, { 48.f, 32.f }, { 48.f, 20.f }, { 52.f, 20.f } } }, // Right

		{ { { 44.f, 20.f }, { 48.f, 20.f }, { 48.f, 16.f }, { 44.f, 16.f } } }, // Top
		{ { { 48.f, 20.f }, { 52.f, 20.f }, { 52.f, 16.f }, { 48.f, 16.f } } }, // Bottom
	};

	TexturedCube legQuads[6] = {
		{ { { 16.f, 32.f }, { 12.f, 32.f }, { 12.f, 20.f }, { 16.f, 20.f } } }, // Back
		{ { { 8.f, 32.f }, { 4.f, 32.f }, { 4.f, 20.f }, { 8.f, 20.f } } }, // Front
		{ { { 12.f, 32.f }, { 8.f, 32.f }, { 8.f, 20.f }, { 12.f, 20.f } } }, // Left
		{ { { 0.f, 32.f }, { 4.f, 32.f }, { 4.f, 20.f }, { 0.f, 20.f } } }, // Right

		{ { { 4.f, 20.f }, { 8.f, 20.f }, { 8.f, 16.f }, { 4.f, 16.f } } }, // Top
		{ { { 8.f, 20.f }, { 12.f, 20.f }, { 12.f, 16.f }, { 8.f, 16.f } } }, // Bottom
	};

	TexturedCube headQuads[6] = {
		{ { { 32.f, 16.f }, { 24.f, 16.f }, { 24.f, 8.f }, { 32.f, 8.f } } }, // Back
		{ { { 16.f, 16.f }, { 8.f, 16.f }, { 8.f, 8.f }, { 16.f, 8.f } } }, // Front
		{ { { 0.f, 16.f }, { 8.f, 16.f }, { 8.f, 8.f }, { 0.f, 8.f } } }, // Left
		{ { { 24.f, 16.f }, { 16.f, 16.f }, { 16.f, 8.f }, { 24.f, 8.f } } }, // Right

		{ { { 8.f, 8.f }, { 16.f, 8.f }, { 16.f, 0.f }, { 8.f, 0.f } } }, // Top
		{ { { 16.f, 8.f }, { 24.f, 8.f }, { 24.f, 0.f }, { 16.f, 0.f } } }, // Bottom
	};

	Torso = ModelBox(glm::vec3(8.0f, 12.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), torsoQuads);
	ArmLeft = ModelBox(glm::vec3(4.0f, 12.0f, 4.0f), glm::vec3(-6.0f, 0.0f, 0.0f), armQuads);
	ArmRight = ModelBox(glm::vec3(4.0f, 12.0f, 4.0f), glm::vec3(6.0f, 0.0f, 0.0f), armQuads);
	LegLeft = ModelBox(glm::vec3(4.0f, 12.0f, 4.0f), glm::vec3(-2.0f, -12.0f, 0.0f), legQuads);
	LegRight = ModelBox(glm::vec3(4.0f, 12.0f, 4.0f), glm::vec3(2.0f, -12.0f, 0.0f), legQuads);
	Head = ModelBox(glm::vec3(8.0f, 8.0f, 8.0f), glm::vec3(0.0f, 10.0f, 0.0f), headQuads);
	HeldItem = ModelPartItem(glm::vec3(0.6f), glm::vec3(5.0f, -13.0f, -5.0f), glm::vec3(0.0f, -90.0f, 45.0f));
	HeldItemBlock = ModelPartItem(glm::vec3(0.3f), glm::vec3(-3.5f, -6.0f, -4.0f), glm::vec3(0.0f, 45.0f, 0.0f));
	HeldItemBlockSprite = ModelPartItem(glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(-2.5f, -7.0f, -1.0f), glm::vec3(-90.0f, 0.0f, 0.0f));
	HeldItemNonTool = ModelPartItem(glm::vec3(0.3f), glm::vec3(-2.5f, -7.0f, -1.0f), glm::vec3(-90.0f, 0.0f, 0.0f));
	//HeldItem.Center.x = 1.0f;

	ArmLeft.Flipped = true;
	ArmLeft.Center.x = 4.0f / 16.0f;
	ArmLeft.Center.y = 4.0f / 16.0f;
	ArmRight.Center.y = 4.0f / 16.0f;

	LegLeft.Center.y = 6.0f / 16.0f;
	LegRight.Center.y = 6.0f / 16.0f;

	Head.Center.y = -4.0f / 16.0f;

	Torso.AddChild(&ArmLeft);
	Torso.AddChild(&ArmRight);
	Torso.AddChild(&LegLeft);
	Torso.AddChild(&LegRight);
	Torso.AddChild(&Head);

	ArmLeft.AddChild(&HeldItem);
	ArmLeft.AddChild(&HeldItemBlock);
	ArmLeft.AddChild(&HeldItemBlockSprite);
	ArmLeft.AddChild(&HeldItemNonTool);

	Texture = Client::GetInstance().pResourceManager->GetHandle(E_TextureResource_Terrain);
}

void ModelHumanoid::Render(VertexProducer& producer, float walkingProgress, float digProgress, float idleProgress, float armsAngle, float legSwingFactor, bool applyWalkToArms)
{
	float walkArmAngle = 60.0f * legSwingFactor;

	armsAngle -= Parameters[1];
	ArmLeft.Rotation = {};

	if (applyWalkToArms)
	{
		ArmLeft.Rotation.x = glm::sin(walkingProgress) * walkArmAngle * Parameters[3];
		ArmRight.Rotation.x = -glm::sin(walkingProgress) * walkArmAngle * Parameters[3];
	}
	else
	{
		ArmLeft.Rotation.x = 0.0f;
		ArmRight.Rotation.x = 0.0f;
	}

	Torso.Offset.y = -11.f / 16.0f;

	LegLeft.Rotation.x = -glm::sin(walkingProgress) * walkArmAngle;
	LegRight.Rotation.x = glm::sin(walkingProgress) * walkArmAngle;
	
	ArmLeft.Rotation.x += Parameters[2];
	ArmLeft.Rotation.x += armsAngle;
	ArmRight.Rotation.x += armsAngle;
	
	ArmLeft.Rotation.z = -(glm::cos(idleProgress / 3.0f) + 1.0f) * 2.0f;
	ArmRight.Rotation.z = (glm::cos(idleProgress / 3.0f) + 1.0f) * 2.0f;

	float pitchRad = glm::radians(Parameters[0]);
	float verticality = glm::pow(glm::abs(glm::sin(pitchRad)), 1);

	Head.Rotation.x = Parameters[0];

	Head.Rotation.y = Parameters[4] * (1.0f - verticality);
	Head.Rotation.z = -glm::sign(Parameters[0]) * Parameters[4] * verticality;

	Torso.Rotation.y = 0.0f;

	if (digProgress > 0.0f)
	{
		float dig = digProgress;
		dig = 1.0f - dig;
		dig *= dig;
		dig *= dig;
		dig = 1.0f - dig;
		float rotateSin = glm::sin(dig * glm::pi<float>());
		float rotateSinHead = glm::sin(digProgress * glm::pi<float>()) * -(glm::radians(Head.Rotation.x) - 0.7f) * 0.75f;
		float bodyRot = (glm::sin(glm::sqrt(digProgress) * 3.141593f * 2.0f) * 0.2f) * 57.29578f;
		ArmLeft.Rotation.x -= (rotateSin * 1.2f + rotateSinHead) * 57.29578f;
		ArmLeft.Rotation.y -= bodyRot * 2.0f;
		ArmLeft.Rotation.z = (glm::sin(-digProgress * glm::pi<float>()) * -0.6f) * 57.29578f;
	}

	if (armsAngle != 0.0f)
	{
		ArmLeft.Rotation.z += -armsAngle / 12.0f;
		ArmRight.Rotation.z += armsAngle / 12.0f;
	}

	Torso.Render(producer, glm::scale(glm::mat4(1.0f), glm::vec3(0.9f)));
}
