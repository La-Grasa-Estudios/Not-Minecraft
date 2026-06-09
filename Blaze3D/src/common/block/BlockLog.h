#pragma once

#include "Block.h"

class BlockLog : public Block
{
public:
	BlockLog(uint8_t id);
	uint8_t GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z) override;
};