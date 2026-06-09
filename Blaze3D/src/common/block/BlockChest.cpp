#include "BlockChest.h"

#include "common/World.h"
#include "common/tile/ChestTileEntity.h"
#include "common/entity/BaseEntity.h"

BlockChest::BlockChest(uint8_t id) : Block(id, 0)
{
	SetTexture(13, 2);
	SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_AXE);
	SetHardness(2.5f);
}

void BlockChest::OnPlace(World* world, BaseEntity* entity, int x, int y, int z, int face, int facing)
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
	world->SetTileEntity(x, y, z, std::make_shared<ChestTileEntity>(world->GetChunk(x, z), x, y, z));
}

void BlockChest::Tick(World* world, int x, int y, int z)
{
	auto block = world->GetBlock(x, y, z);

	auto n1 = world->GetBlock(x - 1, y, z);
	auto n2 = world->GetBlock(x + 1, y, z);
	auto n3 = world->GetBlock(x, y, z - 1);
	auto n4 = world->GetBlock(x, y, z + 1);

	if (n2.Id == BLOCK_CHEST || n1.Id == BLOCK_CHEST)
	{
		int facing = block.Metadata;

		if (facing != 2 && facing != 3)
		{
			facing = 2;
		}

		if (n2.Metadata != block.Metadata && n2.Id == BLOCK_CHEST)
		{
			if (n2.Metadata == 2 || n2.Metadata == 3)
			{
				facing = n2.Metadata;
			}
			else 
			{
				n2.Metadata = facing;
				world->SetBlockNoNotify(x + 1, y, z, n2);
			}
		}
		if (n1.Metadata != block.Metadata && n1.Id == BLOCK_CHEST)
		{
			if (n1.Metadata == 2 || n1.Metadata == 3)
			{
				facing = n1.Metadata;
			}
			else
			{
				n1.Metadata = facing;
				world->SetBlockNoNotify(x - 1, y, z, n1);
			}
		}

		block.Metadata = facing;
	}

	if (n3.Id == BLOCK_CHEST || n4.Id == BLOCK_CHEST)
	{
		int facing = block.Metadata;

		if (facing != 4 && facing != 5)
		{
			facing = 4;
		}

		if (n4.Metadata != block.Metadata && n4.Id == BLOCK_CHEST)
		{
			if (n4.Metadata == 4 || n4.Metadata == 5)
			{
				facing = n4.Metadata;
			}
			else
			{
				n4.Metadata = facing;
				world->SetBlockNoNotify(x, y, z + 1, n4);
			}
		}
		if (n3.Metadata != block.Metadata && n3.Id == BLOCK_CHEST)
		{
			if (n3.Metadata == 4 || n3.Metadata == 5)
			{
				facing = n3.Metadata;
			}
			else
			{
				n3.Metadata = facing;
				world->SetBlockNoNotify(x, y, z - 1, n3);
			}
		}

		block.Metadata = facing;
	}

	world->SetBlockNoNotify(x, y, z, block);
}

static bool IsThereANeighbourChest(World* world, int x, int y, int z)
{
	int neighbours = 0;
	if (world->GetBlock(x - 1, y, z).Id == BLOCK_CHEST)
		neighbours++;
	if (world->GetBlock(x + 1, y, z).Id == BLOCK_CHEST)
		neighbours++;
	if (world->GetBlock(x, y, z - 1).Id == BLOCK_CHEST)
		neighbours++;
	if (world->GetBlock(x, y, z + 1).Id == BLOCK_CHEST)
		neighbours++;
	return neighbours > 0 && world->GetBlock(x, y, z).Id == BLOCK_CHEST;
}

uint8_t BlockChest::GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z)
{
	if (face > 1)
	{
		if (chunk)
		{
			int frontTexture = 27;
			int backTexture = 26;

			auto block = chunk->GetBlockGlobal(x, y, z);

			auto n1 = chunk->GetBlockGlobal(x - 1, y, z);
			auto n2 = chunk->GetBlockGlobal(x + 1, y, z);
			auto n3 = chunk->GetBlockGlobal(x, y, z - 1);
			auto n4 = chunk->GetBlockGlobal(x, y, z + 1);

			if (face == 3 || face == 2)
			{
				if (n1.GetDef() == this)
				{
					if (block.Metadata == 3 && face == 3)
					{
						frontTexture = 42;
					}
					else
					{
						frontTexture = 41;
					}
					if (block.Metadata == 3 && face == 2)
					{
						backTexture = 57;
					}
					else
					{
						backTexture = 58;
					}
				}
				if (n2.GetDef() == this)
				{
					if (block.Metadata == 3 && face == 3)
					{
						frontTexture = 41;
					}
					else
					{
						frontTexture = 42;
					}
					if (block.Metadata == 3 && face == 2)
					{
						backTexture = 58;
					}
					else
					{
						backTexture = 57;
					}
				}
			}

			if (face == 4 || face == 5)
			{
				if (n3.GetDef() == this)
				{
					if (block.Metadata == 5 && face == 5)
					{
						frontTexture = 41;
					}
					else
					{
						frontTexture = 42;
					}
					if (block.Metadata == 5 && face == 4)
					{
						backTexture = 58;
					}
					else
					{
						backTexture = 57;
					}
				}
				if (n4.GetDef() == this)
				{
					if (block.Metadata == 5 && face == 5)
					{
						frontTexture = 42;
					}
					else
					{
						frontTexture = 41;
					}
					if (block.Metadata == 5 && face == 4)
					{
						backTexture = 57;
					}
					else
					{
						backTexture = 58;
					}
				}
			}

			return face == block.Metadata ? frontTexture : backTexture;
		}
		return face == 2 ? 27 : 26;
	}
	return 25;
}

bool BlockChest::UseAt(World* world, BaseEntity* entity, int x, int y, int z)
{
	if (world->IsServerWorld)
	{
		entity->InteractWith(InteractionType::CHEST, x, y, z);
		return true;
	}
	return true;
}

bool BlockChest::CanExistAt(World* world, int x, int y, int z, int faceId, int facing)
{
	int neighbours = 0;
	if (world->GetBlock(x - 1, y, z).GetDef() == this)
		neighbours++;
	if (world->GetBlock(x + 1, y, z).GetDef() == this)
		neighbours++;
	if (world->GetBlock(x, y, z - 1).GetDef() == this)
		neighbours++;
	if (world->GetBlock(x, y, z + 1).GetDef() == this)
		neighbours++;

	return neighbours < 2 && 
		!IsThereANeighbourChest(world, x - 1, y, z) && 
		!IsThereANeighbourChest(world, x + 1, y, z) && 
		!IsThereANeighbourChest(world, x, y, z - 1) && 
		!IsThereANeighbourChest(world, x, y, z + 1);
}
