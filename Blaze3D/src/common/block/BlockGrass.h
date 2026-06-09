#pragma once

#include "Block.h"

class BlockGrass : public Block
{
public:
	BlockGrass(uint8_t id);
	void RandomTick(World* world, int x, int y, int z) override;
	uint8_t GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z) override;

	BlockRenderType GetRenderType() override;
};