#pragma once

#include "Item.h"

class ItemWheatSeed : public Item
{
public:
	float Efficiency = 4.0f;

	ItemWheatSeed(int id);

	bool TryPlace(BaseEntity* entity, World* world, int x, int y, int z, ItemStack& stack);
};