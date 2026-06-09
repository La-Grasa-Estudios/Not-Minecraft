#include "BlockReed.h"

#include "common/World.h"
#include "common/item/Item.h"

BlockReed::BlockReed(uint8_t id) : BlockCrossShaped(id)
{
	m_TextureIndex = 73;
	SetIconIndex(73);
	const float offset = 1.0f / 16.0f;
	SetShape(offset, 0.0f, offset, 1 - offset, 1.0f, 1 - offset);
	SetDrop(338); // Reed Item ID
}

void BlockReed::Tick(World* world, int x, int y, int z)
{
	if (!CanExistAt(world, x, y, z, -1, -1))
	{
		BreakAt(world, NULL, x, y, z, true);
	}
}

void BlockReed::RandomTick(World* world, int x, int y, int z)
{
	if (world->GetBlock(x, y + 1, z).Id == BLOCK_AIR)
	{
		int height = 0;
		for (height = 1; world->GetBlock(x, y - height, z).Id == GetId(); height++);
		if (height < 3)
		{
			world->SetBlock(x, y + 1, z, BLOCK_REED);
		}
	}

	Tick(world, x, y, z);
}

bool BlockReed::CanExistAt(World* world, int x, int y, int z, int faceId, int facing)
{
	auto below = world->GetBlock(x, y - 1, z);

	if (below.Id != BLOCK_GRAVEL && below.Id != BLOCK_SAND && below.Id != BLOCK_REED)
	{
		return false;
	}

	if (below.Id != BLOCK_REED)
	{
		int n1 = world->GetBlock(x + 1, y - 1, z).Id;
		int n2 = world->GetBlock(x - 1, y - 1, z).Id;
		int n3 = world->GetBlock(x, y - 1, z + 1).Id;
		int n4 = world->GetBlock(x, y - 1, z - 1).Id;
		if (n1 != BLOCK_WATER && n2 != BLOCK_WATER && n3 != BLOCK_WATER && n4 != BLOCK_WATER)
		{
			return false;
		}
	}

	return true;
}
