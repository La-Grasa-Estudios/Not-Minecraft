#pragma once

#include "Block.h"

class BlockFire : public Block
{
public:
	BlockFire(uint8_t id);
	void Tick(World* world, int x, int y, int z) override;
	void RandomTick(World* world, int x, int y, int z) override;
	bool CanExistAt(World* world, int x, int y, int z, int faceId, int facing) override;
	void DisplayTick(World* world, int x, int y, int z) override;
	BlockRenderType GetRenderType() override;
};