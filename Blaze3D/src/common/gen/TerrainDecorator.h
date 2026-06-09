#pragma once

class Random;
class CachedChunkAccess;

class TerrainDecorator
{
public:
	static void DecorateTerrain(CachedChunkAccess& access, Random& random, int startX, int startZ);
};