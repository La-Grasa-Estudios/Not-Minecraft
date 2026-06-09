// Based on https://github.com/b3spectacled/modern-beta-fabric/blob/1.16.x/src/main/java/com/bespectacled/modernbeta/world/carver/OldCaveCarver.java

#include "WorldGenCaves.h"

#include "common/Random.h"

#include "common/World.h"

void WorldGenCaves::GenerateInChunk(World* world, Chunk* chunk, Random& random, int regionX, int regionZ)
{
    constexpr int maxCaveCount = 15;
    int caveCount = random.NextInt(random.NextInt(random.NextInt(40) + 1) + 1);
    if (random.NextInt(maxCaveCount) != 0) {
        caveCount = 0;
    }

    int mainChunkX = chunk->PosX;
    int mainChunkZ = chunk->PosZ;

    for (int i = 0; i < caveCount; ++i) {
        float x = regionX * 16 + random.NextInt(16);
        float y = random.NextInt(120) + 8;
        float z = regionZ * 16 + random.NextInt(16);

        int tunnelCount = 1;
        if (random.NextInt(4) == 0) {
            CarveCave(chunk, random, mainChunkX, mainChunkZ, x, y, z);
            tunnelCount += random.NextInt(4);
        }

        for (int j = 0; j < tunnelCount; ++j) {
            float f = random.NextFloat() * 3.141593f * 2.0f;
            float f1 = ((random.NextFloat() - 0.5f) * 2.0f) / 8.0f;
            float tunnelSysWidth = random.NextFloat() * 2.0f + random.NextFloat();

            CarveTunnels(chunk, random, mainChunkX, mainChunkZ, x, y, z, tunnelSysWidth, f, f1, 0, 0, 1.0f);
        }
    }
}

void WorldGenCaves::CarveCave(Chunk* chunk, Random& random, int mainChunkX, int mainChunkZ, float x, float y, float z)
{
    CarveTunnels(chunk, random, mainChunkX, mainChunkZ, x, y, z, 1.0F + random.NextFloat() * 6.0f, 0.0f, 0.0f, -1, -1,
        0.5f);
}

void WorldGenCaves::CarveTunnels(Chunk* chunk, Random& rand, int mainChunkX, int mainChunkZ, float x, float y, float z, float tunnelSysWidth, float f1, float f2, int branch, int branchCount, float tunnelWHRatio)
{
    float f3 = 0.0F;
    float f4 = 0.0F;

    Random random(rand.NextLong());

    if (branchCount <= 0) {
        int someNumMaxStarts = 8 * 16 - 16;
        branchCount = someNumMaxStarts - random.NextInt(someNumMaxStarts / 4);
    }

    bool noStarts = false;
    if (branch == -1) {
        branch = branchCount / 2;
        noStarts = true;
    }

    int randBranch = random.NextInt(branchCount / 2) + branchCount / 4;
    bool vary = random.NextInt(6) == 0;

    for (; branch < branchCount; branch++) {
        float yaw = 1.5f + (float)(glm::sin(((float)branch * 3.141593F) / (float)branchCount)
            * tunnelSysWidth * 1.0F);
        float pitch = yaw * tunnelWHRatio;

        float f5 = glm::cos(f2);
        float f6 = glm::sin(f2);

        x += glm::cos(f1) * f5;
        y += f6;
        z += glm::sin(f1) * f5;

        f2 *= vary ? 0.92F : 0.7F;

        f2 += f4 * 0.1F;
        f1 += f3 * 0.1F;

        f4 *= 0.9F;
        f3 *= 0.75F;

        f4 += (random.NextFloat() - random.NextFloat()) * random.NextFloat() * 2.0F;
        f3 += (random.NextFloat() - random.NextFloat()) * random.NextFloat() * 4.0f;

        if (!noStarts && branch == randBranch && tunnelSysWidth > 1.0F) {
            CarveTunnels(chunk, rand, mainChunkX, mainChunkZ, x, y, z, random.NextFloat() * 0.5f + 0.5f,
                f1 - 1.570796F, f2 / 3.0f, branch, branchCount, 1.0f);
            CarveTunnels(chunk, rand, mainChunkX, mainChunkZ, x, y, z, random.NextFloat() * 0.5f + 0.5f,
                f1 + 1.570796F, f2 / 3.0f, branch, branchCount, 1.0f);
            return;
        }

        if (!noStarts && random.NextInt(4) == 0) {
            continue;
        }

        if (!CanCarveBranch(mainChunkX, mainChunkZ, x, z, branch, branchCount, tunnelSysWidth)) {
            return;
        }

        CarveRegion(chunk, 0, 64, mainChunkX, mainChunkZ, x, y, z, yaw, pitch);

        if (noStarts) {
            break;
        }
    }
}
bool WorldGenCaves::CanCarveBranch(int mainChunkX, int mainChunkZ, float x, float z, int branch, int branchCount, float baseWidth)
{
    float ctrX = mainChunkX * 16 + 8;
    float ctrZ = mainChunkZ * 16 + 8;

    float d1 = x - ctrX;
    float d2 = z - ctrZ;
    float d3 = branchCount - branch;
    float d4 = baseWidth + 2.0F + 16.0f;

    if ((d1 * d1 + d2 * d2) - d3 * d3 > d4 * d4) {
        return false;
    }

    return true;
}

bool WorldGenCaves::CarveRegion(Chunk* chunk, int64_t seed, int seaLevel, int mainChunkX, int mainChunkZ, float x, float y, float z, float yaw, float pitch)
{
    float ctrX = mainChunkX * 16 + 8;
    float ctrZ = mainChunkZ * 16 + 8;

    if ( // Check for valid tunnel starts, I guess? Or to prevent overlap?
        x < ctrX - 16.f - yaw * 2.f || z < ctrZ - 16.f - yaw * 2.f || x > ctrX + 16.f + yaw * 2.f
        || z > ctrZ + 16.f + yaw * 2.f) {
        return false;
    }

    int minX = glm::floor(x - yaw) - mainChunkX * 16 - 1; // Get min and max extents of tunnel, relative to
    // chunk coords
    int maxX = (glm::floor(x + yaw) - mainChunkX * 16) + 1;

    int minY = glm::floor(y - pitch) - 1;
    int maxY = glm::floor(y + pitch) + 1;

    int minZ = glm::floor(z - yaw) - mainChunkZ * 16 - 1;
    int maxZ = (glm::floor(z + yaw) - mainChunkZ * 16) + 1;

    if (minX < 0) {
        minX = 0;
    }
    if (maxX > 16) {
        maxX = 16;
    }

    if (minY < 1) {
        minY = 1;
    }
    if (maxY > 120) {
        maxY = 120;
    }

    if (minZ < 0) {
        minZ = 0;
    }
    if (maxZ > 16) {
        maxZ = 16;
    }

    // Use vanilla methods, for now.
    if (IsRegionUncarvable(chunk, mainChunkX, mainChunkZ, minX, maxX, minY, maxY, minZ, maxZ)) {
        return false;
    }

    for (int relX = minX; relX < maxX; relX++) {

        float scaledRelX = (((float)(relX + mainChunkX * 16) + 0.5f) - x) / yaw;

        for (int relZ = minZ; relZ < maxZ; relZ++) {

            float scaledRelZ = (((float)(relZ + mainChunkZ * 16) + 0.5f) - z) / yaw;
            bool isGrassBlock = false;

            int setY = maxY;

            for (int relY = maxY - 1; relY >= minY; relY--) {
                float scaledRelY = (((float)relY + 0.5f) - y) / pitch;

                if (IsPositionExcluded(scaledRelX, scaledRelY, scaledRelZ, -1)) {
                    auto block = chunk->GetBlock(relX, setY, relZ);

                    if (block.Id == BLOCK_GRASS) {
                        isGrassBlock = true;
                    }

                    if (block.Id == BLOCK_STONE || block.Id == BLOCK_DIRT || block.Id == BLOCK_GRASS || block.Id == BLOCK_NETHERRACK) {
                        if (relY < 10 || (chunk->ChunkWorld->GetDimensionID() == -1 && relY <= 30)) { // Set lava below y = 10
                            chunk->SetBlock(relX, setY, relZ, { BLOCK_LAVA, 8 } );
                        }
                        else {
                            chunk->SetBlock(relX, setY, relZ, BLOCK_AIR);

                            // I believe this replaces carved-out dirt with grass, if block that was removed
                            // was grass.
                            if (isGrassBlock && chunk->GetBlock(relX, setY - 1, relZ)
                                .Id == BLOCK_DIRT) {
                                chunk->SetBlock(relX, setY - 1, relZ, BLOCK_GRASS);
                            }
                        }
                    }
                }
                setY--;
            }

        }
    }

    return true;
}

bool WorldGenCaves::IsRegionUncarvable(Chunk* chunk, int mainChunkX, int mainChunkZ, int relMinX, int relMaxX, int minY, int maxY, int relMinZ, int relMaxZ)
{
    for (int relX = relMinX; relX < relMaxX; relX++) {
        for (int relZ = relMinZ; relZ < relMaxZ; relZ++) {
            for (int relY = maxY + 1; relY >= minY - 1; relY--) {

                if (relY < 0 || relY >= 128) {
                    continue;
                }

                auto block = chunk->GetBlock(relX, relY, relZ);

                if (block.Id == BLOCK_WATER) {
                    return true;
                }

                if (relY != minY - 1 && IsOnBoundary(relMinX, relMaxX, relMinZ, relMaxZ, relX, relZ)) {
                    relY = minY;
                }
            }

        }
    }

    return false;
}

bool WorldGenCaves::IsOnBoundary(int minX, int maxX, int minZ, int maxZ, int relX, int relZ)
{
    return relX != minX && relX != maxX - 1 && relZ != minZ && relZ != maxZ - 1;
}

bool WorldGenCaves::IsPositionExcluded(float scaledRelativeX, float scaledRelativeY, float scaledRelativeZ, int y)
{
    return scaledRelativeY > -0.7f && scaledRelativeX * scaledRelativeX
        + scaledRelativeY * scaledRelativeY + scaledRelativeZ * scaledRelativeZ < 1.0f;
}
