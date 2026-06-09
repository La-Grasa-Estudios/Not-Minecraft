#pragma once

#include "EntityRenderer.h"

class ItemEntity;

class ItemEntityRenderer : public EntityRenderer<ItemEntity>
{
public:
	void Render(ItemEntity* pEntity, riDevice* pDevice) override;
};