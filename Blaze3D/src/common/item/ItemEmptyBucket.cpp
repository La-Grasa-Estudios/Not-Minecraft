#include "ItemEmptyBucket.h"

#include "common/World.h"
#include "common/entity/BaseEntity.h"

ItemEmptyBucket::ItemEmptyBucket(int id) : Item(id)
{
	SetMaxStackSize(16);
}

bool ItemEmptyBucket::TryUseOnLiquid(BaseEntity* entity, World* world, int x, int y, int z, ItemStack& stack)
{
	if (!entity)
		return false;

	auto block = world->GetBlock(x, y, z);

	if (block.Metadata == 8)
	{
		if (block.Id == BLOCK_WATER)
		{
			stack.Shrink(1);
			auto filledBucket = ItemStack{ Item::WaterBucket->GetId(), 1 };
			entity->Inventory.AddStack(filledBucket);
		}
		if (block.Id == BLOCK_LAVA)
		{
			stack.Shrink(1);
			auto filledBucket = ItemStack{ Item::LavaBucket->GetId(), 1 };
			entity->Inventory.AddStack(filledBucket);
		}
		world->SetBlock(x, y, z, BLOCK_AIR);
	}

	return true;
}
