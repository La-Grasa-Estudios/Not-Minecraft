#include "BlockFungus.h"

#include "common/World.h"

BlockFungus::BlockFungus(uint8_t id, uint8_t texture) : BlockCrossShaped(id)
{
	m_TextureIndex = texture;
	SetIconIndex(texture);
	const float offset = 5.0f / 16.0f;
	const float sizeY = 5.0f / 16.0f;
	SetShape(offset, 0.0f, offset, 1 - offset, sizeY, 1 - offset);
}

void BlockFungus::Tick(World* world, int x, int y, int z)
{
	if (!CanExistAt(world, x, y, z, -1, -1))
	{
		BreakAt(world, NULL, x, y, z, true);
	}
}

bool BlockFungus::CanExistAt(World* world, int x, int y, int z, int faceId, int facing)
{
	return BlockCrossShaped::CanExistAt(world, x, y, z, faceId, facing) && world->GetSkyLight(x, y, z) <= 12;
}
