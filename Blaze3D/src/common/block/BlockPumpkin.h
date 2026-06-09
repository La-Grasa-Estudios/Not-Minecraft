#pragma once

#include "Block.h"

class BlockPumpkin : public Block
{
public:
	BlockPumpkin(uint8_t id);
	void OnPlace(World* world, BaseEntity* entity, int x, int y, int z, int face, int facing) override;
	uint8_t GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z) override;
	bool CanExistAt(World* world, int x, int y, int z, int faceId, int facing) override;
};