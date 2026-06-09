#pragma once

#include <cstdint>

class Random;
class CachedChunkAccess;

class WorldGenNetherLava
{
public:
	WorldGenNetherLava();
	bool GenerateAt(CachedChunkAccess& access, Random& random, int startX, int startY, int startZ);
};