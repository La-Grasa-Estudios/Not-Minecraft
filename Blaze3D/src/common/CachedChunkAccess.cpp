#include "CachedChunkAccess.h"

CachedChunkAccess::CachedChunkAccess(World* world, int x, int z, int radiusInChunks)
{
	m_PosX = (x >> 4) << 4;
	m_PosZ = (z >> 4) << 4;
	m_Radius = radiusInChunks;

	int count = (radiusInChunks * 2) + 1;

	assert(count < 32);

	for (int xx = -radiusInChunks; xx <= radiusInChunks; xx++) {
		for (int zz = -radiusInChunks; zz <= radiusInChunks; zz++) {

			int cx = (xx << 4);
			int cz = (zz << 4);

			int ox = (cx >> 4) + radiusInChunks;
			int oz = (cz >> 4) + radiusInChunks;

			cx = x + cx;
			cz = z + cz;

			m_Chunks[oz + ox * count] = world->GetChunk(cx, cz);
		}
	}

	AccessWorld = world;
}

void CachedChunkAccess::SetBlock(int x, int y, int z, BlockStorage block)
{
	auto chunk = GetChunk(x, z);

	if (chunk)
	{
		chunk->SetBlock(x, y, z, block);
		chunk->ComputeColumnHeight(x, z);
	}
}

BlockStorage CachedChunkAccess::GetBlock(int x, int y, int z)
{
	auto chunk = GetChunk(x, z);

	if (chunk)
	{
		return chunk->GetBlock(x, y, z);
	}

	return BLOCK_STONE;
}

int CachedChunkAccess::GetHeight(int x, int z)
{
	auto chunk = GetChunk(x, z);

	if (chunk)
	{
		return chunk->GetHeightLocal(x, z);
	}

	return CHUNK_HEIGHT- 1;
}

int CachedChunkAccess::GetSkyLight(int x, int y, int z)
{
	auto chunk = GetChunk(x, z);

	if (chunk)
	{
		return chunk->GetSkyLightLocal(x, y, z);
	}

	return 0;
}

void CachedChunkAccess::SetSkyLight(int x, int y, int z, int level)
{
	auto chunk = GetChunk(x, z);

	if (chunk)
	{
		chunk->SetSkyLightLocal(x, y, z, level);
	}
}

int CachedChunkAccess::GetBlockLight(int x, int y, int z)
{
	auto chunk = GetChunk(x, z);

	if (chunk)
	{
		return chunk->GetBlockLightLocal(x, y, z);
	}

	return 0;
}

void CachedChunkAccess::SetBlockLight(int x, int y, int z, int level)
{
	auto chunk = GetChunk(x, z);

	if (chunk)
	{
		chunk->SetBlockLightLocal(x, y, z, level);
	}
}

Chunk* CachedChunkAccess::GetChunk(int x, int z)
{
	int cx = ((x >> 4) << 4);
	int cz = ((z >> 4) << 4);

	int i = cx - m_PosX;
	int j = cz - m_PosZ;

	int ox = (i >> 4) + m_Radius;
	int oz = (j >> 4) + m_Radius;

	if (ox > m_Radius * 2 || oz > m_Radius * 2 || ox < 0 || oz < 0) {
		return NULL;
	}

	int count = (m_Radius * 2) + 1;

	return m_Chunks[oz + ox * count];
}