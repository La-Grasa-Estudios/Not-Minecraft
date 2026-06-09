#pragma once

#include "BlockCrossShaped.h"

class BlockReed : public BlockCrossShaped
{
public:
	BlockReed(uint8_t id);
	void Tick(World* world, int x, int y, int z) override;
	void RandomTick(World* world, int x, int y, int z) override;
	bool CanExistAt(World* world, int x, int y, int z, int faceId, int facing) override;
};