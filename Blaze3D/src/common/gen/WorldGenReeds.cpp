#include "WorldGenReeds.h"

#include "common/Random.h"

#include "common/CachedChunkAccess.h"

WorldGenReeds::WorldGenReeds(uint8_t id)
{
	m_BlockId = id;
}

bool WorldGenReeds::GenerateAt(CachedChunkAccess& access, Random& random, int startX, int startY, int startZ)
{
	for (int count = 0; count < 20; count++)
	{
		int x = startX + random.NextInt(4) - random.NextInt(4);
		int y = startY;
		int z = startZ + random.NextInt(4) - random.NextInt(4);
		if (access.GetBlock(x, y, z).Id == BLOCK_AIR && Block::GetBlock(m_BlockId)->CanExistAt(access.AccessWorld, x, y, z, -1, -1))
		{
			int height = 2 + random.NextInt(random.NextInt(3) + 1);

			for (int i = 0; i < height; i++)
			{
				access.SetBlock(x, y + i, z, m_BlockId);
			}
		}
	}

	return true;
}
