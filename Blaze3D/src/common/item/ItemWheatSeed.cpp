#include "ItemWheatSeed.h"

#include "common/World.h"

ItemWheatSeed::ItemWheatSeed(int id) : Item(id)
{
	SetMaxStackSize(64);
	SetIconIndex(9);
}

bool ItemWheatSeed::TryPlace(BaseEntity* entity, World* world, int x, int y, int z, ItemStack& stack)
{
	if (world->GetBlock(x, y - 1, z).Id == BLOCK_FARMLAND)
	{
		world->SetBlock(x, y, z, BLOCK_WHEAT_CROPS);
		stack.Shrink(1);
		return true;
	}

	return false;
}
