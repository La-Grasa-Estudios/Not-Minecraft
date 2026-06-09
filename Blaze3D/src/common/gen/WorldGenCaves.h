#pragma once

#include "WorldGenBase.h"
#include <cstdint>

class WorldGenCaves : public WorldGenBase
{
protected:
	void GenerateInChunk(World* world, Chunk* chunk, Random& random, int regionX, int regionZ) override;
	void CarveCave(Chunk* chunk, Random& random, int mainChunkX, int mainChunkZ, float x, float y, float z);
	void CarveTunnels(Chunk* chunk, Random& rand, int mainChunkX, int mainChunkZ, float x, float y, float z,
		float tunnelSysWidth, float f1, float f2, int branch, int branchCount, float tunnelWHRatio);
	bool CanCarveBranch(int mainChunkX, int mainChunkZ, float x, float z, int branch, int branchCount,
		float baseWidth);
	bool CarveRegion(Chunk* chunk, int64_t seed, int seaLevel,
		int mainChunkX, int mainChunkZ, float x, float y, float z, float yaw, float pitch);
	bool IsRegionUncarvable(Chunk* chunk, int mainChunkX, int mainChunkZ, int relMinX, int relMaxX,
		int minY, int maxY, int relMinZ, int relMaxZ);
	bool IsOnBoundary(int minX, int maxX, int minZ, int maxZ, int relX, int relZ);
	bool IsPositionExcluded(float scaledRelativeX, float scaledRelativeY, float scaledRelativeZ,
		int y);
};