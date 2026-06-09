#pragma once

#include "Item.h"
#include "common/ItemStack.h"

class ItemEmptyBucket : public Item
{
public:
	ItemEmptyBucket(int id);

	bool TryUseOnLiquid(BaseEntity* entity, World* world, int x, int y, int z, ItemStack& stack) override;
};