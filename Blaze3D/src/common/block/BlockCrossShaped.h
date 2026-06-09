#pragma once

#include "Block.h"

class BlockCrossShaped : public Block
{
public:
	BlockCrossShaped(uint8_t id);
	bool CanExistAt(World* world, int x, int y, int z, int faceId, int facing) override;
	BlockRenderType GetRenderType();
};