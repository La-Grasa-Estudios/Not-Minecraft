#include "FurnaceTileEntity.h"

#include "common/World.h"

#include "common/block/Block.h"
#include "common/item/Item.h"
#include "common/entity/ItemEntity.h"
#include "common/nbt/NBT.h"

DECLARE_TILE_ENTITY_CONSTRUCTOR(FurnaceTileEntity, [](Chunk* chunk, TagCompound& tag)
{
	return std::make_shared<FurnaceTileEntity>(chunk);
});

FurnaceTileEntity::FurnaceTileEntity(Chunk* chunk)
{
	this->chunk = chunk;
	CookItemStack = {};
	FuelItemStack = {};
	ResultItemStack = {};
	BurnTicks = 0;
	CookTime = 0;
	CurrentItemBurnTicks = 0;
}

FurnaceTileEntity::FurnaceTileEntity(Chunk* chunk, int x, int y, int z) : FurnaceTileEntity(chunk)
{
	posX = x;
	posY = y;
	posZ = z;
}

void FurnaceTileEntity::Update()
{
	auto blockAt = chunk->GetBlockGlobal(posX, posY, posZ);
	if (blockAt.Id != BLOCK_FURNACE && blockAt.Id != BLOCK_BURNING_FURNACE)
	{
		chunk->SetTileEntity(posX, posY, posZ, NULL);
		auto dropPos = glm::vec3(posX, posY, posZ) + glm::vec3(0.5f);
		chunk->ChunkWorld->AddEntity(std::make_shared<ItemEntity>(chunk->ChunkWorld, dropPos, CookItemStack));
		chunk->ChunkWorld->AddEntity(std::make_shared<ItemEntity>(chunk->ChunkWorld, dropPos, FuelItemStack));
		chunk->ChunkWorld->AddEntity(std::make_shared<ItemEntity>(chunk->ChunkWorld, dropPos, ResultItemStack));
		return;
	}

	int b = GetItemBurnTime(FuelItemStack.Id);
	if (b != 0)
	{
		CurrentItemBurnTicks = b;
	}

	if (BurnTicks > 0)
	{
		BurnTicks--;

		if (blockAt.Id != BLOCK_BURNING_FURNACE)
		{
			blockAt.Id = BLOCK_BURNING_FURNACE;
			chunk->ChunkWorld->SetBlockNoNotify(posX, posY, posZ, blockAt);
		}
	} else if (blockAt.Id != BLOCK_FURNACE)
	{
		blockAt.Id = BLOCK_FURNACE;
		chunk->ChunkWorld->SetBlockNoNotify(posX, posY, posZ, blockAt);
	}

	if (CanSmelt())
	{
		if (BurnTicks <= 0)
		{
			if (FuelItemStack.Id != 0)
			{
				FuelItemStack.Shrink(1);
				BurnTicks = CurrentItemBurnTicks;
			}
		}

		CookTime++;

		if (CookTime == 200)
		{
			if (ResultItemStack.Id == 0)
			{
				ResultItemStack.Id = GetSmeltResult(CookItemStack.Id);
			}
			ResultItemStack.Expand(1);
			CookItemStack.Shrink(1);
			CookTime = 0;
		}
	}
	else
	{
		CookTime = 0;
	}
}

bool FurnaceTileEntity::CanSmelt()
{
	if (CookItemStack.Id == 0)
	{
		return false;
	}
	else
	{
		int result = GetSmeltResult(CookItemStack.Id);
		if (result != 0)
		{
			return (GetItemBurnTime(FuelItemStack.Id) != 0 || BurnTicks > 0) && ResultItemStack.HasSpace(1);
		}
	}
	return false;
}

int FurnaceTileEntity::GetSmeltResult(int id)
{
	switch (id)
	{
	case BLOCK_IRON_ORE:
		return Item::IronIngot->GetId();
	case BLOCK_GOLD_ORE:
		return Item::GoldIngot->GetId();
	case BLOCK_SAND:
		return BLOCK_GLASS;
	case BLOCK_COBBLESTONE:
		return BLOCK_STONE;
	default:
		break;
	}

	if (id == Item::ClayBalls->GetId())
	{
		return Item::ClayBrick->GetId();
	}
	
	if (id == Item::PorkRaw->GetId())
	{
		return Item::PorkCooked->GetId();
	}

	return 0;
}

int FurnaceTileEntity::GetItemBurnTime(int id)
{
	// Blocks
	switch (id)
	{
	case BLOCK_LOG:
	case BLOCK_PLANKS:
		return 300;
	default:
		break;
	}
	// Items
	if (id == Item::Stick->GetId())
		return 100;
	if (id == Item::Coal->GetId())
		return 1600;
	return 0;
}

float FurnaceTileEntity::GetProgress()
{
	return CookTime / 200.0f;
}

float FurnaceTileEntity::GetRemainingBurnTime()
{
	int burnTicks = CurrentItemBurnTicks;
	if (burnTicks < 1)
	{
		burnTicks = 200;
	}
	return BurnTicks / (float)burnTicks;
}

void FurnaceTileEntity::ReadFromNbt(TagCompound& tag)
{
	TileEntity::ReadFromNbt(tag);
	BurnTicks = tag.GetShort("BurnTime");
	CookTime = tag.GetShort("CookTime");
	auto& items = tag.GetTagAs<TagList>("Items");
	for (int i = 0; i < items.Size(); i++)
	{
		auto& tag = items.GetAs<TagCompound>(i);
		if (i == 0)
			CookItemStack.ReadFromTag(tag);
		if (i == 1)
			FuelItemStack.ReadFromTag(tag);
		if (i == 2)
			ResultItemStack.ReadFromTag(tag);
	}
}

void FurnaceTileEntity::WriteToNbt(TagCompound& tag)
{
	TileEntity::WriteToNbt(tag);
	auto items = std::make_shared<TagList>("Items", NBT_TAG_TYPE_COMPOUND);
	items->Add(CookItemStack.WriteToTag());
	items->Add(FuelItemStack.WriteToTag());
	items->Add(ResultItemStack.WriteToTag());
	tag.SetTag(items);
	tag.SetShort("BurnTime", BurnTicks);
	tag.SetShort("CookTime", CookTime);
}
