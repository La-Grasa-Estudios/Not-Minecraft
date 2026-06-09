#pragma once

#include "ChunkProvider.h"

class ChunkProviderHell : public ChunkProvider
{
public:
	ChunkProviderHell(uint64_t seed);
	virtual void ProvideShape(Chunk& chunk, Random& random, uint64_t seed);
	virtual void ProvideSurface(Chunk& chunk, Random& random, uint64_t seed);
	virtual void DecorateSurface(Chunk& chunk, Random& random, uint64_t seed);
	virtual void LightChunk(Chunk& chunk);
};