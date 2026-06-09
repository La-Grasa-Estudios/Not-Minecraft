#include "BlockFarmland.h"

#include "common/Chunk.h"
#include "common/World.h"

constexpr int8_t HYDRATED_VALUE = 7;
constexpr int8_t DEHYDRATED_VALUE = 0;

BlockFarmland::BlockFarmland(uint8_t id) : Block(id, 0)
{
	m_TextureIndex = 2;
	SetRenderNeighbours(true);
	SetShape(0.0f, 0.0f, 0.0f, 1.0f, 15.0f / 16.0f, 1.0f);
	SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_SHOVEL);
	SetHardness(0.5f);
	SetOpacity(0);
	SetIsOpaque(true);
	m_BlocksSideLight = false;
	SetDrop(BLOCK_DIRT);
}

uint8_t BlockFarmland::GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z)
{
	if (face == 1 && chunk)
	{
		return chunk->GetBlockGlobal(x, y, z).Metadata == HYDRATED_VALUE ? 86 : 87;
	}
	return face == 1 ? 86 : 2;
}

bool BlockFarmland::IsFaceRenderable(Chunk* chunk, int x, int y, int z, int face)
{
	return face == 1 || Block::IsFaceRenderable(chunk, x, y, z, face);
}

void BlockFarmland::RandomTick(World* world, int x, int y, int z)
{
	bool foundWaterBlock = false;
	for (int yy = 0; yy < 2 && !foundWaterBlock; yy++)
	{
		for (int xx = -4; xx <= 4 && !foundWaterBlock; xx++)
		{
			for (int zz = -4; zz <= 4 && !foundWaterBlock; zz++)
			{
				if (world->GetBlock(xx + x, yy + y, zz + z).Id == BLOCK_WATER)
				{
					foundWaterBlock = true;
				}
			}
		}
	}

	auto block = world->GetBlock(x, y, z);
	if (foundWaterBlock)
	{
		block.Metadata = HYDRATED_VALUE;
	}
	else
	{
		if (block.Metadata == 0)
		{
			world->SetBlock(x, y, z, BLOCK_DIRT);
			return;
		}
		else
		{
			block.Metadata--;
		}
	}
	world->SetBlock(x, y, z, block);
	Tick(world, x, y, z);
}

void BlockFarmland::Tick(World* world, int x, int y, int z)
{
	auto above = world->GetBlock(x, y + 1, z).GetDef();
	if (above->IsOpaque() || above->IsSolid())
	{
		world->SetBlock(x, y, z, BLOCK_DIRT);
	}
}