#pragma once

#include "ScreenBase.h"
#include "common/ItemStack.h"

class ScreenContainer : public ScreenBase
{
public:
	ScreenContainer(riDevice* device, int x, int y, int z);
	void Render(glm::vec2 sz) override;
	void ReceiveSlotUpdate(int slotId, ItemStack& newItemStack);
private:
	int m_PosX;
	int m_PosY;
	int m_PosZ;
};