#pragma once

#include "Block.h"

class BlockCraftingTable : public Block
{
public:
	BlockCraftingTable(uint8_t id);
	uint8_t GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z) override;
	bool UseAt(World* world, BaseEntity* entity, int x, int y, int z) override;
};