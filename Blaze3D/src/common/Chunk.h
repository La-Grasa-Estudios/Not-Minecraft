#pragma once

#include "block/Block.h"

#include <unordered_map>
#include <memory>
#include <thirdparty/glm/ext.hpp>

#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 128

enum TerrainGenerationStage
{
	TERRAIN_GEN_UNPOPULATED,
	TERRAIN_GEN_SHAPE,
	TERRAIN_GEN_SURFACE,
	TERRAIN_GEN_FEATURES,
	TERRAIN_GEN_LIGHTING,
	TERRAIN_GEN_LIGHTING2,
	TERRAIN_GEN_LIGHTING3,
	TERRAIN_GEN_LIGHTING4,
	TERRAIN_GEN_COMPLETE
};

class World;
class TagCompound;
class TileEntity;

struct HasherVector3Int {
	size_t operator()(const glm::ivec3& v) const;
};

struct Chunk
{
	struct HeightMapEntry
	{
		uint8_t Height;
	};

	World* ChunkWorld;
	int PosX, PosZ, AliveTime, SaveTicks;
	bool InWorldGenQueue = false;
	TerrainGenerationStage GenStage;
	BlockStorage Blocks[CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT];
	uint8_t LightMap[CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT];
	HeightMapEntry HeightMap[CHUNK_WIDTH * CHUNK_WIDTH];
	std::unordered_map<glm::ivec3, std::shared_ptr<TileEntity>, HasherVector3Int> TileEntities;

	Chunk(World* world, int x, int z);

	BlockStorage GetBlock(int x, int y, int z) const;
	BlockStorage GetBlockGlobal(int x, int y, int z) const;
	void SetBlock(int x, int y, int z, BlockStorage block);
	uint8_t GetHeight(int x, int z);
	uint8_t GetHeightLocal(int x, int z);

	void SetSkyLight(int x, int y, int z, int level);
	void SetSkyLightLocal(int x, int y, int z, int level);
	int GetSkyLight(int x, int y, int z);
	int GetSkyLightLocal(int x, int y, int z);

	void SetBlockLight(int x, int y, int z, int level);
	void SetBlockLightLocal(int x, int y, int z, int level);
	int GetBlockLight(int x, int y, int z);
	int GetBlockLightLocal(int x, int y, int z);

	int GetMixedLightLocal(int x, int y, int z);

	void SetTileEntity(int x, int y, int z, std::shared_ptr<TileEntity> entity);
	TileEntity* GetTileEntity(int x, int y, int z);

	bool HasAllNeighboursLoaded();
	bool IsInsideChunk(int x, int y, int z) const;
	void ComputeColumnHeight(int localX, int localZ);

	int GetStartX();
	int GetStartZ();

	void WriteToNbt(TagCompound& root);
	void LoadFromNbt(TagCompound& root);
};