#include "ItemReed.h"

#include "engine/Audio.h"

#include "common/World.h"

ItemReed::ItemReed(int id) : Item(id)
{
	SetMaxStackSize(64);
	SetIconIndex(27);
}

bool ItemReed::TryPlace(BaseEntity* entity, World* world, int x, int y, int z, ItemStack& stack)
{
	if (Block::GetBlock(BLOCK_REED)->CanExistAt(world, x, y, z, -1, -1))
	{
		world->SetBlock(x, y, z, BLOCK_REED);
		world->PlaySoundEffect(Block::GetBlock(BLOCK_REED)->SoundDef->StepSound, 0.25f);
		stack.Shrink(1);
		return true;
	}

	return false;
}
