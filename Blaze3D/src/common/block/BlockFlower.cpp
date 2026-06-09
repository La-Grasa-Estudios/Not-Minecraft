#include "BlockFlower.h"

#include "common/World.h"

BlockFlower::BlockFlower(uint8_t id, uint8_t texture) : BlockCrossShaped(id)
{
	m_TextureIndex = texture;
	SetIconIndex(texture);
	const float offset = 5.0f / 16.0f;
	SetShape(offset, 0.0f, offset, 1 - offset, 1 - offset, 1 - offset);
}

void BlockFlower::Tick(World* world, int x, int y, int z)
{
	if (!CanExistAt(world, x, y, z, -1, -1))
	{
		BreakAt(world, NULL, x, y, z, true);
	}
}

bool BlockFlower::CanExistAt(World* world, int x, int y, int z, int faceId, int facing)
{
	auto below = world->GetBlock(x, y - 1, z);

	if (below.Id != BLOCK_GRASS && below.Id != BLOCK_DIRT)
	{
		return false;
	}

	return BlockCrossShaped::CanExistAt(world, x, y, z, faceId, facing);
}
