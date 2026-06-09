#pragma once

class Random;
class World;
struct Chunk;

class WorldGenBase
{
public:
	void GenerateRegion(World* world, Chunk* chunk);
protected:
	virtual void GenerateInChunk(World* world, Chunk* chunk, Random& random, int regionX, int regionZ) = 0;
	int StructureRegionSizeInChunks = 8;
};