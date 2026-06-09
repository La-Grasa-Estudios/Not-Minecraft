#pragma once

#include "Block.h"

class BlockFurnace : public Block
{
public:
	BlockFurnace(uint8_t id);
	void OnPlace(World* world, BaseEntity* entity, int x, int y, int z, int face, int facing) override;
	void Tick(World* world, int x, int y, int z) override;
	uint8_t GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z) override;
	bool UseAt(World* world, BaseEntity* entity, int x, int y, int z) override;
};