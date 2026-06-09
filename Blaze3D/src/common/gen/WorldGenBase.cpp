#include "WorldGenBase.h"

#include "common/Random.h"

#include "common/World.h"

void WorldGenBase::GenerateRegion(World* world, Chunk* chunk)
{
	Random random(world->Seed);

	auto chunkSeedX = random.NextLong() / 2LL * 2LL + 1LL;
	auto chunkSeedZ = random.NextLong() / 2LL * 2LL + 1LL;

	for (int regionX = -StructureRegionSizeInChunks; regionX <= StructureRegionSizeInChunks; regionX++)
	{
		for (int regionZ = -StructureRegionSizeInChunks; regionZ <= StructureRegionSizeInChunks; regionZ++)
		{
			int chunkX = regionX + chunk->PosX;
			int chunkZ = regionZ + chunk->PosZ;
			random.SetSeed((uint64_t)chunkX * chunkSeedX + (uint64_t)chunkZ * chunkSeedZ ^ world->Seed);
			GenerateInChunk(world, chunk, random, chunkX, chunkZ);
		}
	}
}
