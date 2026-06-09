#include "WorldGenFlowers.h"

#include "common/Random.h"

#include "common/CachedChunkAccess.h"

WorldGenFlowers::WorldGenFlowers(uint8_t id)
{
	m_BlockId = id;
}

bool WorldGenFlowers::GenerateAt(CachedChunkAccess& access, Random& random, int startX, int startY, int startZ)
{
	for (int count = 0; count < 64; count++)
	{
		int x = startX + random.NextInt(8) - random.NextInt(8);
		int y = startY + random.NextInt(4) - random.NextInt(4);
		int z = startZ + random.NextInt(8) - random.NextInt(8);
		if (access.GetBlock(x, y, z).GetDef()->GetId() == 0 && Block::GetBlock(m_BlockId)->CanExistAt(access.AccessWorld, x, y, z, -1, -1))
		{
			access.SetBlock(x, y, z, m_BlockId);
		}
	}

	return true;
}
