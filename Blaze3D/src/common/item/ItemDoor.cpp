#include "ItemDoor.h"

#include "common/World.h"
#include "common/entity/BaseEntity.h"

ItemDoor::ItemDoor(int id, int blockId) : Item(id), BlockId(blockId)
{
	SetMaxStackSize(1);
}

bool ItemDoor::TryPlace(BaseEntity* entity, World* world, int x, int y, int z, ItemStack& stack)
{
	if (world->GetBlock(x, y - 1, z).GetDef()->IsFullyOpaqueBlock())
	{
		world->SetBlock(x, y, z, BlockId);
		Block::GetBlock(BlockId)->OnPlace(world, entity, x, y, z, -1, entity->GetEntityFacingDirection());
		if (world->IsServerWorld)
			world->PlaySound(Block::GetBlock(BlockId)->SoundDef->StepSound, glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f), 0.2f);
		stack.Shrink(1);
		return true;
	}

	return false;
}
