#pragma once

#include "Block.h"

class BlockFallingSand : public Block
{
public:
	BlockFallingSand(uint8_t id, uint8_t texture);
	void Tick(World* world, int x, int y, int z) override;
	void RandomTick(World* world, int x, int y, int z) override;
};