#pragma once

#include "Item.h"

class ItemBow : public Item
{
public:
	ItemBow(int id);

	bool TryUseOnEntity(BaseEntity* entity, World* world, int x, int y, int z);
};