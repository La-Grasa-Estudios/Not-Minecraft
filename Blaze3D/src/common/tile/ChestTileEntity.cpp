#include "ChestTileEntity.h"

#include "common/World.h"

#include "common/block/Block.h"
#include "common/entity/ItemEntity.h"
#include "common/nbt/NBT.h"

DECLARE_TILE_ENTITY_CONSTRUCTOR(ChestTileEntity, [](Chunk* chunk, TagCompound& tag)
{
	return std::make_shared<ChestTileEntity>(chunk);
});


ChestTileEntity::ChestTileEntity(Chunk* chunk)
{
	this->chunk = chunk;
	for (int i = 0; i < 27; i++)
		Inventory[i] = {};
}

ChestTileEntity::ChestTileEntity(Chunk* chunk, int x, int y, int z) : ChestTileEntity(chunk)
{
	posX = x;
	posY = y;
	posZ = z;
}

TileEntity* ChestTileEntity::GetNeighbour()
{
	if (chunk->GetBlockGlobal(posX - 1, posY, posZ).Id == BLOCK_CHEST)
		return chunk->ChunkWorld->GetTileEntity(posX - 1, posY, posZ);
	if (chunk->GetBlockGlobal(posX + 1, posY, posZ).Id == BLOCK_CHEST)
		return chunk->ChunkWorld->GetTileEntity(posX + 1, posY, posZ);
	if (chunk->GetBlockGlobal(posX, posY, posZ - 1).Id == BLOCK_CHEST)
		return chunk->ChunkWorld->GetTileEntity(posX, posY, posZ - 1);
	if (chunk->GetBlockGlobal(posX, posY, posZ + 1).Id == BLOCK_CHEST)
		return chunk->ChunkWorld->GetTileEntity(posX, posY, posZ + 1);

	return nullptr;
}

void ChestTileEntity::Update()
{
	if (chunk->GetBlockGlobal(posX, posY, posZ).Id != BLOCK_CHEST)
	{
		chunk->SetTileEntity(posX, posY, posZ, NULL);
		auto dropPos = glm::vec3(posX, posY, posZ) + glm::vec3(0.5f);
		for (int i = 0; i < 27; i++)
		{
			chunk->ChunkWorld->AddEntity(std::make_shared<ItemEntity>(chunk->ChunkWorld, dropPos, Inventory[i]));
		}
		return;
	}
}

void ChestTileEntity::ReadFromNbt(TagCompound& tag)
{
	TileEntity::ReadFromNbt(tag);
	auto& items = tag.GetTagAs<TagList>("Items");
	for (int i = 0; i < items.Size(); i++)
	{
		auto& tag = items.GetAs<TagCompound>(i);
		Inventory[i].ReadFromTag(tag);
	}
}

void ChestTileEntity::WriteToNbt(TagCompound& tag)
{
	TileEntity::WriteToNbt(tag);
	auto items = std::make_shared<TagList>("Items", NBT_TAG_TYPE_COMPOUND);
	for (int i = 0; i < 27; i++)
	{
		items->Add(Inventory[i].WriteToTag());
	}
	tag.SetTag(items);
}
