#pragma once

#include "BlockCrossShaped.h"

class BlockFungus : public BlockCrossShaped
{
public:
	BlockFungus(uint8_t id, uint8_t texture);
	void Tick(World* world, int x, int y, int z) override;
	bool CanExistAt(World* world, int x, int y, int z, int faceId, int facing) override;
};