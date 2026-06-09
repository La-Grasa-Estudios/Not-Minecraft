#pragma once

#include "Block.h"

class BlockTNT : public Block
{
public:
	BlockTNT(uint8_t id);
	uint8_t GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z) override;
	void OnBlockExploded(World* world, int x, int y, int z);
};