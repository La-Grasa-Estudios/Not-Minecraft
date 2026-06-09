#include "WorldGenCactus.h"

#include "common/Random.h"

#include "common/CachedChunkAccess.h"

WorldGenCactus::WorldGenCactus(uint8_t id)
{
	m_BlockId = id;
}

bool WorldGenCactus::GenerateAt(CachedChunkAccess& access, Random& random, int startX, int startY, int startZ)
{
	for (int count = 0; count < 6; count++)
	{
		int x = startX + random.NextInt(8) - random.NextInt(8);
		int y = startY + random.NextInt(4) - random.NextInt(4);
		int z = startZ + random.NextInt(8) - random.NextInt(8);
		if (access.GetBlock(x, y, z).GetDef()->IsReplaceable())
		{
			int height = 1 + random.NextInt(random.NextInt(3) + 1);
			for (int dy = 0; dy < height; dy++)
			{
				auto block = Block::GetBlock(m_BlockId);
				if (block->CanExistAt(access.AccessWorld, x, y + dy, z, -1, -1))
				{
					access.SetBlock(x, y + dy, z, m_BlockId);
				}
			}
		}
	}

	return true;
}
