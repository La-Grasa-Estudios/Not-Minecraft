#include "ItemFood.h"

#include "common/entity/BaseEntity.h"

ItemFood::ItemFood(int id, int healing) : Item(id)
{
	HealingAmount = healing;
	SetMaxStackSize(1);
}

bool ItemFood::TryUseOnEntity(BaseEntity* entity, World* world, int x, int y, int z)
{
	return entity->AddHealth(HealingAmount);
}
