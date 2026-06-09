#include "BlockCactus.h"

#include "common/World.h"

BlockCactus::BlockCactus(uint8_t id) : Block(id, 0)
{
	const float offset = 1.0f / 16.0f;
	SetHardness(0.4f);
	SetRenderNeighbours(true);
	SetRendersToItself(false);
	SetOpacity(0);
	SetShape(offset, 0.0f, offset, 1.0f - offset, 1.0f, 1.0f - offset);
}

uint8_t BlockCactus::GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z)
{
	return face == 1 ? 69 : (face == 0 ? 71 : 70);
}

bool BlockCactus::IsFaceRenderable(Chunk* chunk, int x, int y, int z, int face)
{
	if (face >= 2)
	{
		return true;
	}
	return Block::IsFaceRenderable(chunk, x, y, z, face);
}

void BlockCactus::RandomTick(World* world, int x, int y, int z)
{
	Tick(world, x, y, z);
}

void BlockCactus::Tick(World* world, int x, int y, int z)
{
	if (!CanExistAt(world, x, y, z, -1, -1))
	{
		BreakAt(world, NULL, x, y, z, true);
	}
}

bool BlockCactus::CanExistAt(World* world, int x, int y, int z, int faceId, int facing)
{
	if (world->GetBlock(x - 1, y, z).Id != 0)
		return false;
	if (world->GetBlock(x + 1, y, z).Id != 0)
		return false;
	if (world->GetBlock(x, y, z - 1).Id != 0)
		return false;
	if (world->GetBlock(x, y, z + 1).Id != 0)
		return false;

	auto below = world->GetBlock(x, y - 1, z).GetDef();
	return below->GetId() == GetId() || below->GetId() == BLOCK_SAND;
}
