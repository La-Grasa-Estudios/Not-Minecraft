#pragma once

#include "Item.h"

class ItemReed : public Item
{
public:
	float Efficiency = 4.0f;

	ItemReed(int id);

	bool TryPlace(BaseEntity* entity, World* world, int x, int y, int z, ItemStack& stack);
};