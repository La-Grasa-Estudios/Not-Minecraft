#include "ItemBucket.h"

#include "common/World.h"

ItemBucket::ItemBucket(int id, int blockId) : Item(id), BlockId(blockId)
{
	SetMaxStackSize(1);
}

bool ItemBucket::TryPlace(BaseEntity* entity, World* world, int x, int y, int z, ItemStack& stack)
{
	if (world->GetBlock(x, y, z).GetDef()->IsReplaceableByPlayer())
	{
		world->SetBlock(x, y, z, BlockStorage(BlockId, 8));
		stack.Id = Item::Bucket->GetId();
		return true;
	}
	return false;
}
