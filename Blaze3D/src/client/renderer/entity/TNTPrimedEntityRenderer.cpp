#include "TNTPrimedEntityRenderer.h"

#include <common/Random.h>
#include <engine/RenderingInterface.h>

#include <client/renderer/block/BlockPreviewRenderer.h>
#include <client/renderer/VertexProducer.h>
#include <client/ResourceManager.h>
#include <client/Client.h>

#include <common/entity/TNTPrimedEntity.h>
#include <thirdparty/glm/ext.hpp>

extern VertexProducer producer;

void TNTPrimedEntityRenderer::Render(TNTPrimedEntity* pEntity, riDevice* pDevice)
{
	producer.Reset();
	producer.SetColor(0xFFFFFFFF);
	producer.SetLightMultiplier(pEntity->GetLightLevel());

	BlockPreviewRenderer::RenderBlock(&producer, Block::GetBlock(BLOCK_TNT));

	auto model = glm::translate(glm::mat4(1.0f), pEntity->Interpolate(pEntity->LastPosition, pEntity->Position, Client::GetInstance().UpdateTimer.a));
	model = glm::translate(model, glm::vec3(-0.5f, 0.0f, -0.5f));

	model = Client::GetInstance().ViewMatrix * model;

	pDevice->SetModelViewMatrix(glm::value_ptr(model));
	pDevice->SetTexturing(true);
	Client::GetInstance().pResourceManager->BindTexture(E_TextureResource_Terrain, pDevice);
	pDevice->Draw(producer.GetVertexPointer(), 2, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
	pDevice->SetTexturing(false);
}
