#include "WorldGenFire.h"

#include "common/Random.h"

#include "common/CachedChunkAccess.h"

WorldGenFire::WorldGenFire(uint8_t id)
{
	m_BlockId = id;
}

bool WorldGenFire::GenerateAt(CachedChunkAccess& access, Random& random, int startX, int startY, int startZ)
{
	for (int count = 0; count < 64; count++)
	{
		int x = startX + random.NextInt(8) - random.NextInt(8);
		int y = startY + random.NextInt(4) - random.NextInt(4);
		int z = startZ + random.NextInt(8) - random.NextInt(8);
		if (access.GetBlock(x, y, z).GetDef()->GetId() == 0 && access.GetBlock(x, y - 1, z).Id == BLOCK_NETHERRACK)
		{
			access.SetBlock(x, y, z, m_BlockId);
		}
	}

	return true;
}
