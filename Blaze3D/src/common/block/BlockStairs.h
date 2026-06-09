#pragma once

#include "Block.h"

class BlockStairs : public Block
{
public:
	BlockStairs(uint8_t id, uint8_t texture);
	void OnPlace(World* world, BaseEntity* entity, int x, int y, int z, int face, int facing) override;
	int GetCollisionBoundingBoxes(World* world, int x, int y, int z, AABB bbs[4]) override;
	bool IsFaceRenderable(Chunk* chunk, int x, int y, int z, int face) override;
	BlockRenderType GetRenderType();
};