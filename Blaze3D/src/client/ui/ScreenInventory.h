#pragma once

#include "ScreenBase.h"
#include "common/ItemStack.h"

class ScreenInventory : public ScreenBase
{
public:
	ScreenInventory(riDevice* device);
	~ScreenInventory();
	void Render(glm::vec2 sz) override;
	ItemStack CraftingGrid[4];
	ItemStack Result;
	ScreenType GetScreenType() override;
	void ReceiveSlotUpdate(int slotId, ItemStack& newItemStack);
};