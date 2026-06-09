#pragma once

#include "Block.h"

class BlockLeaves : public Block
{
public:
	BlockLeaves(uint8_t id, uint8_t texture);
protected:
	int GetDropId() override;
};