#pragma once

#include "Block.h"

class BlockNetherPortal : public Block
{
public:
	BlockNetherPortal(uint8_t id);
	bool IsFaceRenderable(Chunk* chunk, int x, int y, int z, int face) override;
	void Tick(World* world, int x, int y, int z) override;
	AABB GetWorldBoundingBox(World* world, int x, int y, int z);
	BlockRenderType GetRenderType() override;
};