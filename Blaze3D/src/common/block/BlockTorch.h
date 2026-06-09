#pragma once

#include "Block.h"

class BlockTorch : public Block
{
public:
	BlockTorch(uint8_t id);
	void OnPlace(World* world, BaseEntity* entity, int x, int y, int z, int face, int facing) override;
	void Tick(World* world, int x, int y, int z) override;
	void DisplayTick(World* world, int x, int y, int z) override;
	bool CanExistAt(World* world, int x, int y, int z, int faceId, int facing) override;
	AABB GetRaycastBoundingBox(World* world, int x, int y, int z) override;
	BlockRenderType GetRenderType() override;
};