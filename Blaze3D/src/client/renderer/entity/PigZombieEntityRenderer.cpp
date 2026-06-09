#include "PigZombieEntityRenderer.h"

#include <common/entity/PigZombieEntity.h>
#include <common/item/Item.h>
#include <common/block/Block.h>

#include <client/renderer/ItemRenderer.h>
#include <client/renderer/model/ModelRegistry.h>
#include <client/renderer/model/ModelHumanoid.h>
#include <client/renderer/VertexProducer.h>

#include <engine/RenderingInterface.h>

class PlayerEntity;
extern VertexProducer producer;

void PigZombieEntityRenderer::OnModelRender(CreatureEntity* pEntity, riDevice* device, const glm::mat4& model)
{
	auto entity = GetEntity<PigZombieEntity>(pEntity);

	auto renderModel = (ModelHumanoid*)ModelRegistry::GetInstance().GetModel("pigzombie");

	auto itemModel = renderModel->HeldItem.LocalTransform;

	producer.Reset();
	producer.SetLightMultiplier(entity->GetLightLevel());

	ItemStack stack;
	stack.Id = Item::GoldenSword->GetId();
	stack.Amount = 1;

	if (stack.Id)
	{
		if (stack.Id < 256)
		{
			itemModel = renderModel->HeldItemBlock.LocalTransform;
			if (Block::GetBlock(stack.Id)->RendersAsIcon())
			{
				itemModel = renderModel->HeldItemBlockSprite.LocalTransform;
			}
		}
		auto item = Item::GetItem(stack.Id);
		if (item && (item->GetType() == TOOL_TYPE_NONE || item->GetType() == TOOL_TYPE_FLINT_AND_STEEL))
		{
			itemModel = renderModel->HeldItemBlockSprite.LocalTransform;
		}

		itemModel = model * itemModel;

		device->PushMatrixStack();
		ItemRenderer::RenderItemStack(stack, &producer, device, itemModel, ITEM_RENDERER_ENV_WORLD);
		device->PopMatrixStack();
	}
}
