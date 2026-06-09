#pragma once

#include "Block.h"

class BlockCrop : public Block
{
public:
	BlockCrop(uint8_t id, uint8_t texture, uint32_t drop, uint32_t seedDrop);
	void BreakAt(World* world, BaseEntity* entity, int x, int y, int z, bool doDrop) override;
	void RandomTick(World* world, int x, int y, int z) override;
	void Tick(World* world, int x, int y, int z) override;
	bool CanExistAt(World* world, int x, int y, int z, int faceId, int facing) override;
	uint8_t GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z) override;
	bool IsFaceRenderable(Chunk* chunk, int x, int y, int z, int face) override;
	AABB GetRaycastBoundingBox(World* world, int x, int y, int z) override;
	BlockRenderType GetRenderType();
private:
	uint32_t m_CropDrop;
	uint32_t m_SeedDrop;
};