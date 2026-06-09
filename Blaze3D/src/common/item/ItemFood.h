#pragma once

#include "Item.h"

class ItemFood : public Item
{
public:

	int HealingAmount = 0;

	ItemFood(int id, int healing);

	bool TryUseOnEntity(BaseEntity* entity, World* world, int x, int y, int z);
};