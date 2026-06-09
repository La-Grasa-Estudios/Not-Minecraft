#pragma once

#include "BiomeProvider.h"

struct Chunk;
class Random;

class ChunkProvider
{
public:
	ChunkProvider(uint64_t seed);
	virtual void ProvideShape(Chunk& chunk, Random& random, uint64_t seed);
	virtual void ProvideSurface(Chunk& chunk, Random& random, uint64_t seed);
	virtual void DecorateSurface(Chunk& chunk, Random& random, uint64_t seed);
	virtual void LightChunk(Chunk& chunk);
	virtual void PropogateChunk(Chunk& chunk);
	BiomeProvider BiomeGenerator;
};