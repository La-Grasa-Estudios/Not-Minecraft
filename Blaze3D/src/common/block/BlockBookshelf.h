#pragma once

#include "Block.h"

class BlockBookshelf : public Block
{
public:
	BlockBookshelf(uint8_t id);
	uint8_t GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z) override;
};
