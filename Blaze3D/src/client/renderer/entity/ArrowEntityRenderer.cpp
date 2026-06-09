#include "ArrowEntityRenderer.h"

#include <common/Random.h>
#include <engine/RenderingInterface.h>

#include <client/renderer/VertexProducer.h>
#include <client/Client.h>
#include <client/ResourceManager.h>

#include <common/entity/ArrowEntity.h>
#include <thirdparty/glm/ext.hpp>

extern VertexProducer producer;
extern Random g_Random;

void ArrowEntityRenderer::Render(ArrowEntity* pEntity, riDevice* pDevice)
{
	Client::GetInstance().pResourceManager->BindTexture(E_TextureResource_Arrow, pDevice);
	pDevice->SetTexturing(true);

	auto pos = pEntity->Position;
	auto rot = pEntity->Rotation;

	int arrowType = 0;
	float f2 = 0.0f;
	float f3 = 0.5f;
	float f4 = (0 + arrowType * 10) / 32.0f;
	float f5 = (5 + arrowType * 10) / 32.0f;
	float f6 = 0.0f;
	float f7 = 0.15625f;
	float f8 = (5 + arrowType * 10) / 32.0f;
	float f9 = (10 + arrowType * 10) / 32.0f;
	float scale = 0.05625f;

	auto model = glm::translate(glm::mat4(1.0f), pEntity->Interpolate(pEntity->LastPosition, pEntity->Position, Client::GetInstance().UpdateTimer.a));

	model = glm::rotate(model, glm::radians(rot.y - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(rot.z), glm::vec3(0.0f, 0.0f, 1.0f));

	float shake = pEntity->Interpolate(pEntity->m_RenderArrowShake, (float)pEntity->m_ArrowShake, Client::GetInstance().UpdateTimer.a);
	if (shake > 0.0f)
	{
		model = glm::rotate(model, -glm::radians(glm::sin(shake * 3.0f) * shake), glm::vec3(0.0f, 0.0f, 1.0f));
	}

	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(scale));
	model = glm::translate(model, glm::vec3(-4.0f, 0.0f, 0.0f));

	auto viewModel = Client::GetInstance().ViewMatrix * model;

	auto v = -glm::normalize(pEntity->HeadingBeforeHit) * 0.25f;
	auto lastpos = pEntity->Position;
	pEntity->Position += v;
	float l = pEntity->GetLightLevel();
	pEntity->Position = lastpos;

	producer.Reset();
	producer.SetLightMultiplier(l);

	producer.SetColor(1.0f, 1.0f, 1.0f);
	producer.SetNormal(1.0f, 0.0f, 0.0f);
	producer.AddVertex(-7.0f, -2.0f, -2.0f, f6, f8);
	producer.AddVertex(-7.0f, -2.0f, 2.0f, f7, f8);
	producer.AddVertex(-7.0f, 2.0f, 2.0f, f7, f9);
	producer.AddVertex(-7.0f, 2.0f, -2.0f, f6, f9);
	producer.SetNormal(-1.0f, 0.0f, 0.0f);
	producer.AddVertex(-7.0f, -2.0f, -2.0f, f6, f8);
	producer.AddVertex(-7.0f, -2.0f, 2.0f, f7, f8);
	producer.AddVertex(-7.0f, 2.0f, 2.0f, f7, f9);
	producer.AddVertex(-7.0f, 2.0f, -2.0f, f6, f9);

	pDevice->PushMatrixStack();
	pDevice->SetModelViewMatrix(glm::value_ptr(viewModel));

	pDevice->Draw(producer.GetVertexPointer(), 2, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());

	for (int i = 0; i < 4; i++)
	{
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//auto normal = glm::vec3(0.0f, 0.0f, 1.0f) * glm::mat3(model);
		producer.Reset();
		producer.SetLightMultiplier(l);
		producer.SetColor(1.0f, 1.0f, 1.0f);
		producer.SetNormal(0.0f, 0.0f, 1.0f);

		producer.AddVertex(-8.0f, -2.0f, 0.0f, f2, f4);
		producer.AddVertex(8.0f, -2.0f, 0.0f, f3, f4);
		producer.AddVertex(8.0f, 2.0f, 0.0f, f3, f5);
		producer.AddVertex(-8.0f, 2.0f, 0.0f, f2, f5);

		viewModel = Client::GetInstance().ViewMatrix * model;
		pDevice->SetModelViewMatrix(glm::value_ptr(viewModel));
		pDevice->Draw(producer.GetVertexPointer(), 2, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
	}

	pDevice->PopMatrixStack();
}
