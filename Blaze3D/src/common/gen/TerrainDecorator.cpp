#include "TerrainDecorator.h"

#include "common/Random.h"
#include "common/World.h"
#include "common/world/BiomeBase.h"
#include "common/world/ChunkProvider.h"
#include "common/noise/PerlinNoise.h"
#include "common/CachedChunkAccess.h"
#include "common/item/Item.h"

#include "WorldGenMinable.h"
#include "WorldGenFlowers.h"
#include "WorldGenReeds.h"
#include "WorldGenPumpkin.h"
#include "WorldGenCactus.h"
#include "WorldGenClay.h"

#include "common/tile/ChestTileEntity.h"

void TerrainDecorator::DecorateTerrain(CachedChunkAccess& access, Random& random, int startX, int startZ)
{
    constexpr int triesToGenerateClayVein = 10;

    constexpr int triesToGenerateDirtVein = 20;
    constexpr int triesToGenerateGravelVein = 10;
    constexpr int triesToGenerateCoalVein = 20;
    constexpr int triesToGenerateIronVein = 20;
    constexpr int triesToGenerateGoldVein = 2;
    constexpr int triesToGenerateDiamondVein = 1;

    constexpr int countOfDirtBlocksPerVein = 32;
    constexpr int countOfGravelBlocksPerVein = 32;
    constexpr int countOfCoalBlocksPerVein = 16;
    constexpr int countOfIronBlocksPerVein = 8;
    constexpr int countOfGoldBlocksPerVein = 8;
    constexpr int countOfDiamondBlocksPerVein = 8;

    constexpr int triesToGenerateYellowFlowerPatch = 2;
    constexpr int triesToGenerateReedPatch = 10;

    for (int count = 0; count < triesToGenerateClayVein; count++)
    {
        int x = startX + random.NextInt(CHUNK_WIDTH);
        int y = random.NextInt(CHUNK_HEIGHT);
        int z = startZ + random.NextInt(CHUNK_WIDTH);
        WorldGenClay(BLOCK_CLAY).GenerateAt(access, random, x, y, z);
    }

    for (int count = 0; count < triesToGenerateDirtVein; count++)
    {
        int x = startX + random.NextInt(CHUNK_WIDTH);
        int y = random.NextInt(CHUNK_HEIGHT);
        int z = startZ + random.NextInt(CHUNK_WIDTH);
        WorldGenMinable(BLOCK_DIRT, countOfDirtBlocksPerVein).GenerateAt(access, random, x, y, z);
    }

    for (int count = 0; count < triesToGenerateGravelVein; count++)
    {
        int x = startX + random.NextInt(CHUNK_WIDTH);
        int y = random.NextInt(CHUNK_HEIGHT);
        int z = startZ + random.NextInt(CHUNK_WIDTH);
        WorldGenMinable(BLOCK_GRAVEL, countOfGravelBlocksPerVein).GenerateAt(access, random, x, y, z);
    }

    for (int count = 0; count < triesToGenerateCoalVein; count++)
    {
        int x = startX + random.NextInt(CHUNK_WIDTH);
        int y = random.NextInt(CHUNK_HEIGHT);
        int z = startZ + random.NextInt(CHUNK_WIDTH);
        WorldGenMinable(BLOCK_COAL_ORE, countOfCoalBlocksPerVein).GenerateAt(access, random, x, y, z);
    }

    for (int count = 0; count < triesToGenerateIronVein; count++)
    {
        int x = startX + random.NextInt(CHUNK_WIDTH);
        int y = random.NextInt(64);
        int z = startZ + random.NextInt(CHUNK_WIDTH);
        WorldGenMinable(BLOCK_IRON_ORE, countOfIronBlocksPerVein).GenerateAt(access, random, x, y, z);
    }

    for (int count = 0; count < triesToGenerateGoldVein; count++)
    {
        int x = startX + random.NextInt(CHUNK_WIDTH);
        int y = random.NextInt(32);
        int z = startZ + random.NextInt(CHUNK_WIDTH);
        WorldGenMinable(BLOCK_GOLD_ORE, countOfGoldBlocksPerVein).GenerateAt(access, random, x, y, z);
    }

    for (int count = 0; count < triesToGenerateDiamondVein; count++)
    {
        int x = startX + random.NextInt(CHUNK_WIDTH);
        int y = random.NextInt(16);
        int z = startZ + random.NextInt(CHUNK_WIDTH);
        WorldGenMinable(BLOCK_DIAMOND_ORE, countOfDiamondBlocksPerVein).GenerateAt(access, random, x, y, z);
    }

    for (int count = 0; count < triesToGenerateYellowFlowerPatch; count++)
    {
        int x = startX + random.NextInt(CHUNK_WIDTH) + 8;
        int y = random.NextInt(128);
        int z = startZ + random.NextInt(CHUNK_WIDTH) + 8;
        WorldGenFlowers(BLOCK_YELLOW_FLOWER).GenerateAt(access, random, x, y, z);
    }

    if (random.NextInt(2) == 0)
    {
        int x = startX + random.NextInt(CHUNK_WIDTH) + 8;
        int y = random.NextInt(128);
        int z = startZ + random.NextInt(CHUNK_WIDTH) + 8;
        WorldGenFlowers(BLOCK_RED_ROSE).GenerateAt(access, random, x, y, z);
    }

    /*
    if (random.NextInt(2) == 0)
    {
        int x = startX + random.NextInt(CHUNK_WIDTH) + 8;
        int y = random.NextInt(128);
        int z = startZ + random.NextInt(CHUNK_WIDTH) + 8;
        WorldGenFlowers(BLOCK_BROWN_MUSHROOM).GenerateAt(access, random, x, y, z);
    }

    if (random.NextInt(4) == 0)
    {
        int x = startX + random.NextInt(CHUNK_WIDTH) + 8;
        int y = random.NextInt(128);
        int z = startZ + random.NextInt(CHUNK_WIDTH) + 8;
        WorldGenFlowers(BLOCK_RED_MUSHROOM).GenerateAt(access, random, x, y, z);
    }
    */

    for (int count = 0; count < triesToGenerateReedPatch; count++)
    {
        int x = startX + random.NextInt(CHUNK_WIDTH) + 8;
        int y = random.NextInt(128);
        int z = startZ + random.NextInt(CHUNK_WIDTH) + 8;
        WorldGenReeds(BLOCK_REED).GenerateAt(access, random, x, y, z);
    }

    if (random.NextInt(32) == 0)
    {
        int x = startX + random.NextInt(CHUNK_WIDTH) + 8;
        int y = random.NextInt(128);
        int z = startZ + random.NextInt(CHUNK_WIDTH) + 8;
        WorldGenPumpkin(BLOCK_PUMPKIN).GenerateAt(access, random, x, y, z);
    }

    int biomeX = startX;
    int biomeZ = startZ;

    access.AccessWorld->GetChunkProvider().BiomeGenerator.GenerateValuesForChunk(startX, startZ);

    auto biome = access.AccessWorld->GetChunkProvider().BiomeGenerator.GetBiomeForColumn(biomeX & 0xf, biomeZ & 0xf);

    for (int i = 0; i < biome->CactiPerChunk; i++)
    {
        int x = startX + random.NextInt(CHUNK_WIDTH) + 8;
        int y = random.NextInt(128);
        int z = startZ + random.NextInt(CHUNK_WIDTH) + 8;
        WorldGenCactus(BLOCK_CACTUS).GenerateAt(access, random, x, y, z);
    }

    float treeScale = 0.5f;
    auto treeNoise = PerlinNoiseOctaves(random, 8);
    int treeCount = (int)((treeNoise.SampleOctaves((float)startX * treeScale, (float)startZ * treeScale) / 8.0f + random.NextFloat() * 4.0f + 4.0f) / 3.0f);
    if (biome->TreesPerChunk == -1)
    {
        treeCount = 0;
    }
    else
    {
        treeCount = treeCount + biome->TreesPerChunk;
    }
    if (random.NextInt(10) == 0)
    {
        treeCount++;
    }

    for (int i = 0; i < treeCount; i++)
    {
        int x = startX + random.NextInt(CHUNK_WIDTH) + 8;
        int z = startZ + random.NextInt(CHUNK_WIDTH) + 8;
        int y = access.GetHeight(x, z) - 1;

        BlockStorage block = access.GetBlock(x, y, z);

        if (block.Id == BLOCK_GRASS)
        {
            int woodID = BLOCK_LOG;
            int leavesID = BLOCK_LEAVES;

            access.SetBlock(x, y, z, 3);

            y += 1;

            int trunkHeight = random.NextInt(3) + 4;

            for (int i = 0; i < trunkHeight; i++) {

                access.SetBlock(x, y + i, z, woodID);

                if (i == trunkHeight - 3 || i == trunkHeight - 2) {
                    for (int n1 = -2; n1 < 3; n1++) {
                        for (int n2 = -2; n2 < 3; n2++) {
                            bool shouldPlace = true;
                            if (n1 == -2 && n2 == -2) {
                                if (random.NextInt(10) > 5) shouldPlace = false;
                            }
                            if (n1 == -2 && n2 == 2) {
                                if (random.NextInt(10) > 5) shouldPlace = false;
                            }
                            if (n1 == 2 && n2 == 2) {
                                if (random.NextInt(10) > 5) shouldPlace = false;
                            }
                            if (n1 == 2 && n2 == -2) {
                                if (random.NextInt(10) > 5) shouldPlace = false;
                            }

                            if (shouldPlace) {
                                if (access.GetBlock(x + n1, y + i, z + n2).Id == 0)
                                    access.SetBlock(x + n1, y + i, z + n2, leavesID);
                            }

                        }
                    }
                }
                else if (i == trunkHeight - 1) {
                    if (access.GetBlock(x + 1, y + i, z).Id == 0)
                        access.SetBlock(x + 1, y + i, z, leavesID);
                    if (access.GetBlock(x - 1, y + i, z).Id == 0)
                        access.SetBlock(x - 1, y + i, z, leavesID);
                    if (access.GetBlock(x, y + i, z + 1).Id == 0)
                        access.SetBlock(x, y + i, z + 1, leavesID);
                    if (access.GetBlock(x, y + i, z - 1).Id == 0)
                        access.SetBlock(x, y + i, z - 1, leavesID);
                    if (access.GetBlock(x + 1, y + i, z + 1).Id == 0)
                        access.SetBlock(x + 1, y + i, z + 1, leavesID);
                    if (access.GetBlock(x - 1, y + i, z - 1).Id == 0)
                        access.SetBlock(x - 1, y + i, z - 1, leavesID);
                    if (access.GetBlock(x - 1, y + i, z + 1).Id == 0)
                        access.SetBlock(x - 1, y + i, z + 1, leavesID);
                    if (access.GetBlock(x + 1, y + i, z - 1).Id == 0)
                        access.SetBlock(x + 1, y + i, z - 1, leavesID);
                }
            }

            access.SetBlock(x + 1, y + trunkHeight, z, leavesID);
            access.SetBlock(x - 1, y + trunkHeight, z, leavesID);
            access.SetBlock(x, y + trunkHeight, z + 1, leavesID);
            access.SetBlock(x, y + trunkHeight, z - 1, leavesID);
            access.SetBlock(x, y + trunkHeight, z, leavesID);
        }
    }

    for (int x = 0; x < 16; x++)
    {
        for (int z = 0; z < 16; z++)
        {
            biome = access.AccessWorld->GetChunkProvider().BiomeGenerator.GetBiomeForColumn(x, z);

            int cx = x + startX;
            int cz = z + startZ;
            int y = access.GetHeight(cx, cz) - 1;

            float temp = access.AccessWorld->GetChunkProvider().BiomeGenerator.GetTemperature(x, z) - (float)(y - 64) / 64.0f * 0.3f;

            if (biome->Snowy || temp < 0.5f)
            {
                if (access.GetBlock(cx, y, cz).GetDef()->IsSolid() && access.GetBlock(cx, y, cz).GetDef()->IsOpaque())
                {
                    access.SetBlock(cx, y + 1, cz, BLOCK_SNOW);
                }
            }
        }
    }

    int spawnChunkX = (access.AccessWorld->SpawnX / 16) * 16;
    int spawnChunkZ = (access.AccessWorld->SpawnZ / 16) * 16;

    if (startX == spawnChunkX && startZ == spawnChunkZ)
    {
        int height = access.GetHeight(startX, startZ);

        int itemCount = 0;
        int itemIndex = 0;
        int chestIndex = 0;

        while (itemIndex < 128)
        {
            access.SetBlock(startX - chestIndex, height, startZ, BLOCK_CHEST);
            Block::GetBlock(BLOCK_CHEST)->OnPlace(access.AccessWorld, NULL, startX - chestIndex, height, startZ, 0, 0);
            access.AccessWorld->ScheduleUpdate(startX - chestIndex, height, startZ, 0, TICK_TYPE_NORMAL);
            auto chest = static_cast<ChestTileEntity*>(access.AccessWorld->GetTileEntity(startX - chestIndex, height, startZ));

            for (int i = 0; i < 27; i++)
            {
                while (!Block::GetBlock(itemIndex))
                {
                    itemIndex++;
                    if (itemIndex >= 128)
                    {
                        break;
                    }
                }
                if (Block::GetBlock(itemIndex))
                {
                    chest->Inventory[i].Id = itemIndex;
                    chest->Inventory[i].Amount = 64;
                }
                itemIndex++;
            }

            chestIndex++;

            if (chestIndex % 2 == 0)
            {
                chestIndex++;
            }

        }

        itemIndex = 0;

        while (itemIndex <= 92)
        {
            access.SetBlock(startX - chestIndex, height, startZ, BLOCK_CHEST);
            Block::GetBlock(BLOCK_CHEST)->OnPlace(access.AccessWorld, NULL, startX - chestIndex, height, startZ, 0, 0);
            access.AccessWorld->ScheduleUpdate(startX - chestIndex, height, startZ, 0, TICK_TYPE_NORMAL);
            auto chest = static_cast<ChestTileEntity*>(access.AccessWorld->GetTileEntity(startX - chestIndex, height, startZ));

            for (int i = 0; i < 27; i++)
            {
                while (!Item::GetItem(itemIndex + 256))
                {
                    itemIndex++;
                    if (itemIndex > 92)
                    {
                        break;
                    }
                }
                if (Item::GetItem(itemIndex + 256))
                {
                    chest->Inventory[i].Id = Item::GetItem(itemIndex + 256)->GetId();
                    chest->Inventory[i].Amount = Item::GetItem(itemIndex + 256)->GetMaxStackSize();
                }
                itemIndex++;
            }

            chestIndex++;

            if (chestIndex % 2 == 0)
            {
                chestIndex++;
            }

        }

    }
}