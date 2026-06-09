#pragma once

#include "World.h"

class CachedChunkAccess
{
public:

	CachedChunkAccess(World* world, int x, int z, int radiusInChunks);
	void SetBlock(int x, int y, int z, BlockStorage block);
	BlockStorage GetBlock(int x, int y, int z);
	int GetHeight(int x, int z);
	int GetSkyLight(int x, int y, int z);
	void SetSkyLight(int x, int y, int z, int level);
	int GetBlockLight(int x, int y, int z);
	void SetBlockLight(int x, int y, int z, int level);

	World* AccessWorld;

private:

	Chunk* GetChunk(int x, int z);

	Chunk* m_Chunks[32];
	int m_PosX;
	int m_PosZ;
	int m_Radius;
};