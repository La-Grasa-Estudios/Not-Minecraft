#include "BlockLeaves.h"

#include "common/Random.h"

BlockLeaves::BlockLeaves(uint8_t id, uint8_t texture) : Block(id, texture)
{
}

int BlockLeaves::GetDropId()
{
	return Random::GlobalRand.NextInt(20) == 0 ? BLOCK_SAPLING : 0;
}
