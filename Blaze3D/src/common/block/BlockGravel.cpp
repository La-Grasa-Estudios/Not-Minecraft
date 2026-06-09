#include "BlockGravel.h"
#include "common/Random.h"

BlockGravel::BlockGravel(uint8_t id, uint8_t texture) : BlockFallingSand(id, texture)
{
}

int BlockGravel::GetDropId()
{
	return Random::GlobalRand.NextInt(10) == 0 ? 318 : GetId();
}
