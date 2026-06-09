#include "WorldGenMinable.h"

#include "common/Random.h"

#include "common/CachedChunkAccess.h"

#include <thirdparty/glm/ext.hpp>

WorldGenMinable::WorldGenMinable(uint8_t id, int count)
{
	m_BlockId = id;
	m_CountOfBlocks = count;
}

bool WorldGenMinable::GenerateAt(CachedChunkAccess& access, Random& random, int startX, int startY, int startZ)
{
    float angle = random.NextFloat() * glm::pi<float>();

    float startXOffset = (startX + 8) + glm::sin(angle) * m_CountOfBlocks / 8.0F;
    float endXOffset = (startX + 8) - glm::sin(angle) * m_CountOfBlocks / 8.0F;
    float startZOffset = (startZ + 8) + glm::cos(angle) * m_CountOfBlocks / 8.0F;
    float endZOffset = (startZ + 8) - glm::cos(angle) * m_CountOfBlocks / 8.0F;

    float startYOffset = startY + random.NextInt(3) + 2;
    float endYOffset = startY + random.NextInt(3) + 2;

    for (int blockIndex = 0; blockIndex <= m_CountOfBlocks; ++blockIndex) {
        float centerX = startXOffset + (endXOffset - startXOffset) * blockIndex / m_CountOfBlocks;
        float centerY = startYOffset + (endYOffset - startYOffset) * blockIndex / m_CountOfBlocks;
        float centerZ = startZOffset + (endZOffset - startZOffset) * blockIndex / m_CountOfBlocks;

        float randomScale = random.NextFloat() * m_CountOfBlocks / 16.0f;
        float radiusXZ = (glm::sin(blockIndex * glm::pi<float>() / m_CountOfBlocks) + 1.0F) * randomScale + 1.0f;
        float radiusY = (glm::sin(blockIndex * glm::pi<float>() / m_CountOfBlocks) + 1.0F) * randomScale + 1.0f;

        for (int x = (int)(centerX - radiusXZ / 2.0f); x <= (int)(centerX + radiusXZ / 2.0f); ++x) {
            for (int y = (int)(centerY - radiusY / 2.0f); y <= (int)(centerY + radiusY / 2.0f); ++y) {
                for (int z = (int)(centerZ - radiusXZ / 2.0f); z <= (int)(centerZ + radiusXZ / 2.0f); ++z) {
                    float dx = (x + 0.5f - centerX) / (radiusXZ / 2.0f);
                    float dy = (y + 0.5f - centerY) / (radiusY / 2.0f);
                    float dz = (z + 0.5f - centerZ) / (radiusXZ / 2.0f);

                    if (dx * dx + dy * dy + dz * dz < 1.0f &&
                        access.GetBlock(x, y, z).Id == BLOCK_STONE) {
                        access.SetBlock(x, y, z, m_BlockId);
                    }
                }
            }
        }
    }
    return true;
}
