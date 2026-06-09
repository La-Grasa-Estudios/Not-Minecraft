#include "CreatureEntityRenderer.h"

#include <engine/RenderingInterface.h>

#include <client/renderer/RenderHelper.h>
#include <client/renderer/VertexProducer.h>
#include <client/renderer/model/ModelRegistry.h>
#include <client/ResourceManager.h>
#include <client/Client.h>

#include <common/entity/CreatureEntity.h>
#include <common/entity/PlayerEntity.h>
#include <thirdparty/glm/ext.hpp>

extern VertexProducer producer;

void CreatureEntityRenderer::Render(CreatureEntity* pEntity, riDevice* pDevice)
{
	auto renderModel = ModelRegistry::GetInstance().GetModel(pEntity->GetCreatureModel());

	auto dead = pEntity->Interpolate((float)pEntity->m_LastDeadTicks, (float)pEntity->m_DeadTicks, Client::GetInstance().UpdateTimer.a) / 10.0f;
	dead = glm::min(dead, 1.0f);
	dead = glm::pow(1.0f - dead, 2);
	dead = 1.0f - dead;
	auto pos = pEntity->Interpolate(pEntity->LastPosition,pEntity->Position, Client::GetInstance().UpdateTimer.a);
	auto rot = pEntity->Interpolate(pEntity->LastRotation,pEntity->Rotation, Client::GetInstance().UpdateTimer.a);
	auto damage = pEntity->Interpolate((float)pEntity->LastDamageTicks, (float)pEntity->DamageTicks, Client::GetInstance().UpdateTimer.a);
	auto walk = fmod(pEntity->Interpolate(pEntity->LastWalkDist, pEntity->WalkDist, Client::GetInstance().UpdateTimer.a) * 2.0f, glm::pi<float>() * 2.0f) + (damage * !pEntity->IsGrounded);
	auto swing = pEntity->Interpolate(pEntity->m_LastLegSwing, pEntity->m_LegSwing, Client::GetInstance().UpdateTimer.a);

	auto model = glm::translate(glm::mat4(1.0f), pos);

	model = glm::rotate(model, glm::radians(-rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(0.0f, -pEntity->HeightOffset, 0.0f));
	model = glm::rotate(model, glm::radians(dead * 90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(0.0f, pEntity->HeightOffset, 0.0f));
	model = Client::GetInstance().ViewMatrix * model;

	pDevice->PushMatrixStack();
	pDevice->SetModelViewMatrix(glm::value_ptr(model));
	pDevice->SetTexturing(true);

	pDevice->SetTexture(renderModel->Texture, 0);
	pDevice->SetCullMode(RI_CULL_MODE_NONE);

	producer.Reset();
	producer.SetLightMultiplier(pEntity->GetLightLevel());
	producer.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	renderModel->Render(producer, walk, 0.0f, pEntity->TicksAlive + Client::GetInstance().UpdateTimer.a, 0.0f, swing, true);

	pDevice->Draw(producer.GetVertexPointer(), 2, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());

	if (pEntity->DamageTicks > 0 || pEntity->m_InterpDeadTicks > 0)
	{
		producer.Reset();
		producer.SetLightMultiplier(pEntity->GetLightLevel());
		producer.SetColor(1.0f, 0.0f, 0.0f, 0.4f);
		renderModel->Render(producer, walk, 0.0f, pEntity->TicksAlive + Client::GetInstance().UpdateTimer.a, 0.0f, swing, true);
		pDevice->SetTexturing(false);
		pDevice->Draw(producer.GetVertexPointer(), 2, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
	}

	OnModelRender(pEntity, pDevice, model);

	pDevice->SetCullMode(RI_CULL_MODE_BACK);
	pDevice->SetTexturing(false);
	pDevice->PopMatrixStack();

	if (pEntity->IsBurning())
	{
		model = glm::translate(glm::mat4(1.0f), pos);
		//model = glm::translate(model, glm::vec3(0.0f, HeightOffset / 2.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-Client::GetInstance().Player->Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		model = Client::GetInstance().ViewMatrix * model;

		producer.Reset();
		producer.SetLightMultiplier(pEntity->GetLightLevel());
		producer.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

		auto minUv = glm::vec2(240, 16) / 256.0f;
		auto maxUv = glm::vec2(256, 32) / 256.0f;

		producer.SetTranslation(0.0f, 0.0f, 100.0f);
		RenderHelper::RenderCenteredTexturedRect({  }, { pEntity->BoundingBoxWidth, pEntity->BoundingBoxHeight * 2.0f }, maxUv, minUv, producer);

		pDevice->PushMatrixStack();
		pDevice->SetModelViewMatrix(glm::value_ptr(model));
		pDevice->SetCullMode(RI_CULL_MODE_NONE);
		Client::GetInstance().pResourceManager->BindTexture(E_TextureResource_Terrain, pDevice);
		pDevice->SetTexturing(true);
		pDevice->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
		pDevice->SetTexturing(false);
		pDevice->SetCullMode(RI_CULL_MODE_BACK);
		pDevice->PopMatrixStack();
	}
}

void CreatureEntityRenderer::RenderEntityModel(CreatureEntity* pEntity, riDevice* device, const glm::mat4& model, const glm::vec4& color, int renderFlags)
{
	auto renderModel = ModelRegistry::GetInstance().GetModel(pEntity->GetCreatureModel());

	auto damage = pEntity->Interpolate((float)pEntity->LastDamageTicks, (float)pEntity->DamageTicks, Client::GetInstance().UpdateTimer.a);
	auto walk = fmod(pEntity->WalkDist * 2.0f, glm::pi<float>() * 2.0f) + (damage * !pEntity->IsGrounded);
	auto swing = pEntity->m_LegSwing;

	device->PushMatrixStack();
	device->SetModelViewMatrix((float*)glm::value_ptr(model));
	device->SetTexturing(renderFlags & CREATURE_RENDER_FLAG_TEXTURING);

	device->SetTexture(renderModel->Texture, 0);
	device->SetCullMode(RI_CULL_MODE_NONE);

	producer.Reset();

	if (renderFlags & CREATURE_RENDER_FLAG_WORLD_LIGHTING)
		producer.SetLightMultiplier(pEntity->GetLightLevel());

	if (!(renderFlags & CREATURE_RENDER_FLAG_LIGHTING))
	{
		device->SetLighting(false);
	}

	producer.SetColor(color.r, color.b, color.b, color.a);
	renderModel->Render(producer, walk, 0.0f, pEntity->TicksAlive + Client::GetInstance().UpdateTimer.a, 0.0f, swing, true);

	device->Draw(producer.GetVertexPointer(), 2, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());

	device->SetCullMode(RI_CULL_MODE_BACK);
	device->SetTexturing(false);
	device->PopMatrixStack();

	if (!(renderFlags & CREATURE_RENDER_FLAG_LIGHTING))
	{
		device->SetLighting(true);
	}
}