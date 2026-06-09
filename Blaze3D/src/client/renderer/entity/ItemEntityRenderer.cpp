#include "ItemEntityRenderer.h"

#include <common/Random.h>
#include <engine/Time.h>
#include <engine/RenderingInterface.h>

#include <client/renderer/VertexProducer.h>
#include <client/renderer/ItemRenderer.h>
#include <client/Client.h>

#include <common/entity/ItemEntity.h>
#include <thirdparty/glm/ext.hpp>

extern VertexProducer producer;
extern Random g_Random;

void ItemEntityRenderer::Render(ItemEntity* pEntity, riDevice* pDevice)
{
	if (!pEntity->m_Stack.Id)
		return;

	pEntity->m_LiveTime += Time::DeltaTime;

	float l = pEntity->GetLightLevel();

	Random rand(pEntity->m_UUID);

	auto model = glm::translate(glm::mat4(1.0f), pEntity->Interpolate(pEntity->LastPosition, pEntity->Position, Client::GetInstance().UpdateTimer.a));
	model = glm::translate(model, glm::vec3(0.0f, glm::sin(pEntity->m_LiveTime * 3.14f) * 0.1f + 0.1f, 0.0f));

	if (pEntity->m_Stack.Id >= 256)
	{
		model = glm::scale(model, glm::vec3(0.3f));
	}
	else
	{
		model = glm::scale(model, glm::vec3(0.25f));
	}

	model = glm::rotate(model, pEntity->m_LiveTime, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-0.5f));

	model = Client::GetInstance().ViewMatrix * model;

	for (int i = 0; i < pEntity->m_Stack.Amount && i < 8; i++)
	{
		producer.Reset();
		producer.SetLightMultiplier(l);
		if (i > 0)
		{
			float x = (rand.NextFloat() - 0.5f);
			float y = (rand.NextFloat() - 0.5f);
			float z = (rand.NextFloat() - 0.5f);
			producer.SetTranslation(x, y, z);
		}

		pDevice->PushMatrixStack();
		pDevice->SetModelViewMatrix(glm::value_ptr(model));
		pDevice->SetTexturing(true);
		ItemRenderer::RenderItemStack(pEntity->m_Stack, &producer, pDevice, model, ITEM_RENDERER_ENV_WORLD);
		pDevice->SetTexturing(false);
		pDevice->PopMatrixStack();
	}
}
