#include "BlockFurnace.h"

#include "common/World.h"
#include "common/entity/BaseEntity.h"
#include "common/tile/FurnaceTileEntity.h"

BlockFurnace::BlockFurnace(uint8_t id) : Block(id, 0)
{
	SetTexture(13, 2);
	SetEffectiveTool(TOOL_TIER_WOOD, TOOL_TYPE_PICKAXE);
	SetHardness(2.0f);
}

void BlockFurnace::OnPlace(World* world, BaseEntity* entity, int x, int y, int z, int face, int facing)
{
	auto block = world->GetBlock(x, y, z);

	static constexpr int inverseFacingLut[4] =
	{
		1,
		0,
		3,
		2
	};

	block.Metadata = inverseFacingLut[facing] + 2; // Skip down and up faces
	world->SetBlockNoNotify(x, y, z, block);
	world->SetTileEntity(x, y, z, std::make_shared<FurnaceTileEntity>(world->GetChunk(x, z), x, y, z));
}

void BlockFurnace::Tick(World* world, int x, int y, int z)
{
}

uint8_t BlockFurnace::GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z)
{
	if (face > 1)
	{
		if (chunk)
		{
			auto block = chunk->GetBlockGlobal(x, y, z);
			return face == block.Metadata ? (GetId() == BLOCK_BURNING_FURNACE ? 61 : 44) : 45;
		}
		return face == 2 ? (GetId() == BLOCK_BURNING_FURNACE ? 61 : 44) : 45;
	}
	return 1;
}

bool BlockFurnace::UseAt(World* world, BaseEntity* entity, int x, int y, int z)
{
	if (world->IsServerWorld)
	{
		entity->InteractWith(InteractionType::FURNACE, x, y, z);
		return true;
	}
	return true;
}
