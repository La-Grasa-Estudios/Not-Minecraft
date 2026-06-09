#pragma once

#include "TileEntity.h"
#include "common/ItemStack.h"

class ChestTileEntity : public TileEntity
{
public:
	ChestTileEntity(Chunk* chunk);
	ChestTileEntity(Chunk* chunk, int x, int y, int z);

	TileEntity* GetNeighbour();

	void Update();

	void ReadFromNbt(TagCompound& tag);
	void WriteToNbt(TagCompound& tag);

	ItemStack Inventory[27];
};
