#pragma once

#include "Block.h"

class BlockFarmland : public Block
{
public:
	BlockFarmland(uint8_t id);
	uint8_t GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z) override;
	bool IsFaceRenderable(Chunk* chunk, int x, int y, int z, int face) override;
	void RandomTick(World* world, int x, int y, int z) override;
	void Tick(World* world, int x, int y, int z) override;
};