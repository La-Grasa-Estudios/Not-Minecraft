#include "WorldGenGlowstone.h"

#include "common/Random.h"

#include "common/CachedChunkAccess.h"

WorldGenGlowstone::WorldGenGlowstone(uint8_t id)
{
	m_BlockId = id;
}

bool WorldGenGlowstone::GenerateAt(CachedChunkAccess& access, Random& random, int startX, int startY, int startZ)
{
	if (access.GetBlock(startX, startY, startZ).Id != BLOCK_AIR)
	{
		return false;
	}
	else if (access.GetBlock(startX, startY + 1, startZ).Id != BLOCK_NETHERRACK)
	{
		return false;
	}
	else
	{
		access.SetBlock(startX, startY, startZ, BLOCK_GLOWSTONE);
		for (int i = 0; i < 1500; i++)
		{
			int x = startX + random.NextInt(8) - random.NextInt(8);
			int y = startY - random.NextInt(12);
			int z = startZ + random.NextInt(8) - random.NextInt(8);

			if (access.GetBlock(x, y, z).Id == 0) {
				int nbNeighbours = 0;

				for (int face = 0; face < 6; ++face) {
					int block = 0;
					if (face == 0) {
						block = access.GetBlock(x - 1, y, z).Id;
					}

					if (face == 1) {
						block = access.GetBlock(x + 1, y, z).Id;
					}

					if (face == 2) {
						block = access.GetBlock(x, y - 1, z).Id;
					}

					if (face == 3) {
						block = access.GetBlock(x, y + 1, z).Id;
					}

					if (face == 4) {
						block = access.GetBlock(x, y, z - 1).Id;
					}

					if (face == 5) {
						block = access.GetBlock(x, y, z + 1).Id;
					}

					if (block == m_BlockId) {
						++nbNeighbours;
					}
				}

				if (nbNeighbours == 1) {
					access.SetBlock(x, y, z, m_BlockId);
				}
			}
		}
	}

	return true;
}
