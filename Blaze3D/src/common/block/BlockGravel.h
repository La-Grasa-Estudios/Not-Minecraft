#pragma once

#include "BlockFallingSand.h"

class BlockGravel : public BlockFallingSand
{
public:
	BlockGravel(uint8_t id, uint8_t texture);
protected:
	int GetDropId() override;
};