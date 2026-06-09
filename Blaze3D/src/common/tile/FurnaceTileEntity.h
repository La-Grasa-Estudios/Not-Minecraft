#pragma once

#include "TileEntity.h"
#include "common/ItemStack.h"

class FurnaceTileEntity : public TileEntity
{
public:
	FurnaceTileEntity(Chunk* chunk);
	FurnaceTileEntity(Chunk* chunk, int x, int y, int z);

	void Update();

	bool CanSmelt();
	int GetSmeltResult(int id);
	int GetItemBurnTime(int id);
	float GetProgress();
	float GetRemainingBurnTime();

	void ReadFromNbt(TagCompound& tag);
	void WriteToNbt(TagCompound& tag);

	int BurnTicks;
	int CurrentItemBurnTicks;
	int CookTime;

	ItemStack FuelItemStack;
	ItemStack CookItemStack;
	ItemStack ResultItemStack;
};
