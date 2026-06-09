#pragma once

#include "Item.h"

class ItemDoor : public Item
{
public:
	float Efficiency = 4.0f;

	ItemDoor(int id, int blockId);

	bool TryPlace(BaseEntity* entity, World* world, int x, int y, int z, ItemStack& stack);

	int BlockId;
};