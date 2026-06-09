#include "WorldGenPumpkin.h"

#include "common/Random.h"

#include "common/CachedChunkAccess.h"

WorldGenPumpkin::WorldGenPumpkin(uint8_t id)
{
	m_BlockId = id;
}

bool WorldGenPumpkin::GenerateAt(CachedChunkAccess& access, Random& random, int startX, int startY, int startZ)
{
	for (int count = 0; count < 64; count++)
	{
		int x = startX + random.NextInt(8) - random.NextInt(8);
		int y = startY + random.NextInt(4) - random.NextInt(4);
		int z = startZ + random.NextInt(8) - random.NextInt(8);
		if (access.GetBlock(x, y, z).GetDef()->IsReplaceable() && access.GetBlock(x, y - 1, z).Id == BLOCK_GRASS && Block::GetBlock(m_BlockId)->CanExistAt(access.AccessWorld, x, y, z, -1, -1))
		{
			access.SetBlock(x, y, z, BlockStorage(m_BlockId, (uint8_t)random.NextInt(4) + 2));
		}
	}

	return true;
}
