#pragma once

#include "ScreenBase.h"
#include "common/ItemStack.h"

class ScreenCrafting : public ScreenBase
{
public:
	ScreenCrafting(riDevice* device);
	~ScreenCrafting();
	void Render(glm::vec2 sz) override;
	ItemStack CraftingGrid[9];
	ItemStack Result;
	ScreenType GetScreenType() override;
	void ReceiveSlotUpdate(int slotId, ItemStack& newItemStack);
};