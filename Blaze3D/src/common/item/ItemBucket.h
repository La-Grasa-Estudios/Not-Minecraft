#pragma once

#include "Item.h"

class ItemBucket : public Item
{
public:
	ItemBucket(int id, int blockId);

	bool TryPlace(BaseEntity* entity, World* world, int x, int y, int z, ItemStack& stack);
	int BlockId;
};