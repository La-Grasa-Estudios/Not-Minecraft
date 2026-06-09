#include "WorldGenNetherLava.h"

#include "common/Random.h"

#include "common/block/BlockLiquid.h"
#include "common/CachedChunkAccess.h"

WorldGenNetherLava::WorldGenNetherLava()
{
}

bool WorldGenNetherLava::GenerateAt(CachedChunkAccess& access, Random& random, int startX, int startY, int startZ)
{
	if (access.GetBlock(startX, startY + 1, startZ).Id != BLOCK_NETHERRACK) {
		return false;
	}
	else if (access.GetBlock(startX, startY, startZ).Id != BLOCK_AIR && access.GetBlock(startX, startY, startZ).Id != BLOCK_NETHERRACK) {
		return false;
	}
	{
		int stoneNeighbours = 0;
		if (access.GetBlock(startX - 1, startY, startZ).Id == BLOCK_NETHERRACK) {
			++stoneNeighbours;
		}

		if (access.GetBlock(startX + 1, startY, startZ).Id == BLOCK_NETHERRACK) {
			++stoneNeighbours;
		}

		if (access.GetBlock(startX, startY, startZ - 1).Id == BLOCK_NETHERRACK) {
			++stoneNeighbours;
		}

		if (access.GetBlock(startX, startY, startZ + 1).Id == BLOCK_NETHERRACK) {
			++stoneNeighbours;
		}

		if (access.GetBlock(startX, startY - 1, startZ).Id == BLOCK_NETHERRACK) {
			++stoneNeighbours;
		}

		int airNeighbours = 0;
		if (access.GetBlock(startX - 1, startY, startZ).Id == 0) {
			++airNeighbours;
		}

		if (access.GetBlock(startX + 1, startY, startZ).Id == 0) {
			++airNeighbours;
		}

		if (access.GetBlock(startX, startY, startZ - 1).Id == 0) {
			++airNeighbours;
		}

		if (access.GetBlock(startX, startY, startZ + 1).Id == 0) {
			++airNeighbours;
		}

		if (access.GetBlock(startX, startY - 1, startZ).Id == 0) {
			++airNeighbours;
		}

		if (stoneNeighbours == 4 && airNeighbours == 1) {
			access.SetBlock(startX, startY, startZ, { BLOCK_LAVA, 8 });
			static_cast<BlockLiquid*>(Block::GetBlock(BLOCK_LAVA))->InstantUpdate(access.AccessWorld, startX, startY, startZ);
		}
	}
	return true;
}
