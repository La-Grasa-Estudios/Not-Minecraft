#pragma once

#include "Item.h"

class ItemFlintAndSteel : public Item
{
public:
	ItemFlintAndSteel(int id);

	bool TryPlace(BaseEntity* entity, World* world, int x, int y, int z);
};