#include "BlockPumpkin.h"

#include "common/World.h"

BlockPumpkin::BlockPumpkin(uint8_t id) : Block(id, 0)
{
}

void BlockPumpkin::OnPlace(World* world, BaseEntity* entity, int x, int y, int z, int face, int facing)
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
}

uint8_t BlockPumpkin::GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z)
{
	if (face > 1)
	{
		if (chunk)
		{
			auto block = chunk->GetBlockGlobal(x, y, z);
			return face == block.Metadata ? (GetId() == BLOCK_JACK_O_LANTERN ? 120 : 119) : 118;
		}
		return face == 2 ? (GetId() == BLOCK_JACK_O_LANTERN ? 120 : 119) : 118;
	}
	return 102;
}

bool BlockPumpkin::CanExistAt(World* world, int x, int y, int z, int faceId, int facing)
{
	auto block = world->GetBlock(x, y, z).GetDef();
	return (block->GetId() == 0 || block->GetLiquidType() != 0 || block->IsReplaceable()) && world->GetBlock(x, y - 1, z).GetDef()->IsSolid();
}
