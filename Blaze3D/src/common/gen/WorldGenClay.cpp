#include "WorldGenClay.h"

#include "common/Random.h"

#include "common/CachedChunkAccess.h"

WorldGenClay::WorldGenClay(uint8_t id)
{
	m_BlockId = id;
}

bool WorldGenClay::GenerateAt(CachedChunkAccess& access, Random& random, int startX, int startY, int startZ)
{
	const int nbBlocks = 32;

	if (access.GetBlock(startX, startY, startZ).Id != BLOCK_WATER)
	{
		return false;
	}

	int amount = random.NextInt(nbBlocks - 2) + 2;
	int sizeY = 1;

	for (int x = startX - amount; x <= startX + amount; x++)
	{
		for (int z = startZ - amount; z <= startZ + amount; z++)
		{
			int dx = x - startX;
			int dz = z - startZ;

			if (dx * dx + dz * dz > amount * amount)
			{
				continue;
			}

			for (int y = startY - sizeY; y <= startY + sizeY; y++)
			{
				int id = access.GetBlock(x, y, z).Id;

				if (id == BLOCK_DIRT || id == BLOCK_SAND || id == BLOCK_GRAVEL)
				{
					if (access.GetBlock(x, y + 1, z).Id == BLOCK_WATER)
					{
						access.SetBlock(x, y, z, m_BlockId);
					}
				}
			}
		}
	}

	return true;
}
