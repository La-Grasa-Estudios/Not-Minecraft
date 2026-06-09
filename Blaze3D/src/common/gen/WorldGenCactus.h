#pragma once

#include <cstdint>

class Random;
class CachedChunkAccess;

class WorldGenCactus
{
public:
	WorldGenCactus(uint8_t id);
	bool GenerateAt(CachedChunkAccess& access, Random& random, int startX, int startY, int startZ);
private:
	uint8_t m_BlockId;
};