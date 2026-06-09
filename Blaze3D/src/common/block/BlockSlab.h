#pragma once

#include "Block.h"

class BlockSlab : public Block
{
public:
	BlockSlab(uint8_t id, uint8_t textureTop, uint8_t textureSide);
	uint8_t GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z) override;
	bool IsFaceRenderable(Chunk* chunk, int x, int y, int z, int face) override;
private:
	uint8_t m_TopTexture;
};