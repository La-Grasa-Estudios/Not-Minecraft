#include "World.h"
#include "CachedChunkAccess.h"

#include "common/Random.h"
#include "engine/System.h"

#include "thirdparty/glm/ext.hpp"

#include "gen/TerrainDecorator.h"

#include "entity/BaseEntity.h"
#include "entity/PlayerEntity.h"
#include "tile/TileEntity.h"

#include "world/MobSpawner.h"
#include "world/ChunkProvider.h"
#include "world/Explosion.h"
#include "nbt/NBT.h"
#include "path/PathFinder.h"

class PlayerEntity;

const uint32_t ENTITY_CAP = 128;

#ifdef __wii__
#define WORLD_SIZE 16
#define LOAD_DISTANCE 2
#define LIMITED_WORLD
#else
#define WORLD_SIZE 30
#define LOAD_DISTANCE 16
#endif

World::World(const std::string& path)
{
	m_SaveManager = SaveManager(path);

	m_ChunkMap = {};
	Seed = 0;

	m_IsWorldThreadRunning = true;
	m_IsWorldActive = true;

	std::thread t([this] {
		while (m_IsWorldActive || !m_JobQueue.empty())
		{
			while (!m_JobQueue.empty())
			{
				auto task = m_JobQueue.front();
				{
					m_TaskMutex.lock();
					if (m_JobQueue.empty())
					{
						m_TaskMutex.unlock();
						break;
					}
					m_JobQueue.pop();
					m_TaskMutex.unlock();
				}
				task();
			}
			std::unique_lock lock(wakeMutex);
			m_WakeCondition.wait(lock);
			sysYield();
		}
		m_IsWorldThreadRunning = false;
		});
	t.detach();

	InitWorld();

	IsServerWorld = true;
}

World::~World()
{
	WorldAccess = NULL;
	m_ChunkGenQueue = {};
	m_TickList = {};
	m_DelayedTickList = {};
	m_Entities.clear();
	m_AliveEntities.clear();
	m_CollidingEntities.clear();
	m_ChunkMap.clear();
	delete m_ChunkProvider;
}

void World::Generate()
{
	auto listener = WorldAccess;
	//WorldAccess = NULL;

	Random random{};
	Seed = random.NextLong() + random.NextLong();

	if (GetDimensionID() == 0)
		SearchValidSpawn();

#ifdef LIMITED_WORLD

	PreGenerate(WORLD_SIZE + 8);

	Save([this](float p)
		{
			WorldAccess->RenderLoadingScreenWithBar("Saving chunks", p);
		});

	return;

#endif

	int genDistance = 16;
#ifdef __wii__
	genDistance = 6;
#endif

	RequestChunksSurroundingArea(SpawnX, SpawnZ, genDistance, true, [this](float p)
	{
		WorldAccess->RenderLoadingScreenWithBar("Generating Spawn", p);
	});

	int count = 0;

	int targetChunkCount = (genDistance - 3) * 2 + 1;
	targetChunkCount *= targetChunkCount;

	//targetChunkCount -= (int)(targetChunkCount / 1.5f);

	while (!AreChunksLoaded(SpawnX / 16, SpawnZ / 16, genDistance - 3, TERRAIN_GEN_COMPLETE))
	{
		count = 0;
		ProcessWorldGenQueue();
		for (auto& kv : m_ChunkMap)
		{
			auto chunk = kv.second.get();

			if (kv.second->GenStage == TERRAIN_GEN_COMPLETE)
			{
				count++;
			}

			if (chunk->GenStage != TERRAIN_GEN_COMPLETE)
			{
				if (chunk->GenStage == TERRAIN_GEN_LIGHTING && AreChunksLoaded(chunk->PosX, chunk->PosZ, 1, TERRAIN_GEN_LIGHTING))
				{
					chunk->GenStage = TERRAIN_GEN_LIGHTING2;
					PushTask([this, chunk]
						{
							for (int x = 0; x < 16; x++)
							{
								for (int z = 0; z < 16; z++)
								{
									chunk->ComputeColumnHeight(x, z);
									for (int y = 0; y < CHUNK_HEIGHT; y++)
									{
										chunk->SetSkyLightLocal(x, y, z, chunk->GetHeightLocal(x, z) <= y ? 15 : 0);
									}
								}
							}
							chunk->GenStage = TERRAIN_GEN_LIGHTING3;
						}, IsServerWorld);
				}
				if (chunk->GenStage == TERRAIN_GEN_LIGHTING3 && AreChunksLoaded(chunk->PosX, chunk->PosZ, 1, TERRAIN_GEN_LIGHTING3))
				{
					chunk->GenStage = TERRAIN_GEN_LIGHTING4;
					PushTask([this, chunk] {
						CachedChunkAccess access(this, chunk->GetStartX(), chunk->GetStartZ(), 1);

						for (int x = 0; x < 16; x++)
						{
							for (int z = 0; z < 16; z++)
							{
								int xx = chunk->GetStartX() + x;
								int zz = chunk->GetStartZ() + z;

								UpdateSkyLightColumn(xx, zz, &access);

								for (int y = 0; y < CHUNK_HEIGHT; y++)
								{
									if (chunk->GetBlockLightLocal(x, y, z) < chunk->GetBlock(x, y, z).GetDef()->GetLightEmitted())
									{
										UpdateBlockLight(chunk->GetStartX() + x, y, chunk->GetStartZ() + z, &access);
									}
								}
							}
						}

						if (WorldAccess)
						{
							WorldAccess->SetColumnDirty(chunk->GetStartX(), chunk->GetStartZ());
							WorldAccess->SetColumnDirty(chunk->GetStartX() - 16, chunk->GetStartZ());
							WorldAccess->SetColumnDirty(chunk->GetStartX() + 16, chunk->GetStartZ());
							WorldAccess->SetColumnDirty(chunk->GetStartX(), chunk->GetStartZ() - 16);
							WorldAccess->SetColumnDirty(chunk->GetStartX(), chunk->GetStartZ() + 16);
							WorldAccess->SetColumnDirty(chunk->GetStartX() - 16, chunk->GetStartZ() - 16);
							WorldAccess->SetColumnDirty(chunk->GetStartX() + 16, chunk->GetStartZ() + 16);
							WorldAccess->SetColumnDirty(chunk->GetStartX() - 16, chunk->GetStartZ() + 16);
							WorldAccess->SetColumnDirty(chunk->GetStartX() + 16, chunk->GetStartZ() - 16);
						}

						chunk->GenStage = TERRAIN_GEN_COMPLETE;
						}, IsServerWorld);
				}
			}
		}
		WorldAccess->RenderLoadingScreenWithBar("Finishing Up", count / (float)(targetChunkCount));
	}

	Save([this](float p)
	{
		WorldAccess->RenderLoadingScreenWithBar("Saving chunks", p);
	});

	while (!m_ChunkGenQueue.empty())
		m_ChunkGenQueue.pop();

	while (!m_JobQueue.empty())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	m_ChunkMap.clear();
	WorldAccess = listener;
	
}

void World::PreGenerate(int radius)
{
	GetChunkProvider();

	int loadAreaSize = 1;

	int xPos = -radius, zPos = -radius;
	int count = (radius * 2 + 1) * (radius * 2 + 1);
	int idx = 0;

	loadAreaSize = 1;
	xPos = -radius;

	while (!m_JobQueue.empty())
	{
		WorldAccess->RenderLoadingScreenWithBar("Saving", (float)(count - m_JobQueue.size()) / count);
	}

	// Terrain & Features
	while (xPos <= radius)
	{
		zPos = -radius;
		while (zPos <= radius)
		{
			int x = xPos + SpawnX / 16;
			int z = zPos + SpawnZ / 16;
			for (int rdx = -loadAreaSize; rdx <= loadAreaSize; rdx++)
			{
				for (int rdz = -loadAreaSize; rdz <= loadAreaSize; rdz++)
				{
					int chunkX = x + rdx;
					int chunkZ = z + rdz;
					if (auto c = GetChunk(chunkX * 16, chunkZ * 16); c)
					{
						c->AliveTime = 0;
					}
					else
					{
						auto chunk = std::make_shared<Chunk>(this, chunkX, chunkZ);

						if (!m_SaveManager.Load(chunk.get()))
						{
							Random random(Seed);
							m_ChunkProvider->ProvideShape(*chunk, random, Seed);
						}

						m_ChunkMap[GetChunkCoord(chunkX, chunkZ)] = chunk;
					}
				}
			}

			idx++;
			if (idx % 100 == 0)
			{
				char buffer[64];
				snprintf(buffer, 64, "Generating Features %i/%i", idx, count);
				WorldAccess->RenderLoadingScreenWithBar(buffer, (float)idx / count);
			}

			auto chunk = GetChunk(x * 16, z * 16);
			auto random = Random(Seed);

			auto l1 = (random.NextInt() / 2) * 2 + 1;
			auto l2 = (random.NextInt() / 2) * 2 + 1;

			auto chunkSeed = chunk->PosX * l1 + chunk->PosZ * l2;

			random.SetSeed(chunkSeed);

			CachedChunkAccess chunkAccess(this, chunk->GetStartX(), chunk->GetStartZ(), 1);

			for (int x = 0; x < 16; x++)
			{
				for (int z = 0; z < 16; z++)
				{
					chunk->ComputeColumnHeight(x, z);
				}
			}

			TerrainDecorator decorator;
			decorator.DecorateTerrain(chunkAccess, random, chunk->GetStartX(), chunk->GetStartZ());

			chunk->GenStage = TERRAIN_GEN_LIGHTING;

			for (auto it = m_ChunkMap.begin(); it != m_ChunkMap.end(); )
			{
				it->second->AliveTime++;

				if (it->second->AliveTime > 10)
				{
					auto chunk = it->second;

					if (m_JobQueue.size() < 32 && WORLD_SIZE >= 27)
					{
						PushTask([chunk, this]
							{
								m_SaveManager.Save(chunk.get());
							}, IsServerWorld);
					}
					else
					{
						m_SaveManager.Save(chunk.get());
					}

					it = m_ChunkMap.erase(it);
				}
				else
				{
					++it;
				}
			}

			zPos += 1;
		}
		xPos += 1;
	}

	while (!m_JobQueue.empty())
	{
		WorldAccess->RenderLoadingScreenWithBar("Saving", (float)(count - m_JobQueue.size()) / count);
	}

	// Lighting

	radius -= 1;
	count = (radius * 2 + 1) * (radius * 2 + 1);
	xPos = -radius;
	idx = 0;

	while (xPos <= radius)
	{
		zPos = -radius;
		while (zPos <= radius)
		{
			int x = xPos + SpawnX / 16;
			int z = zPos + SpawnZ / 16;
			for (int rdx = -loadAreaSize; rdx <= loadAreaSize; rdx++)
			{
				for (int rdz = -loadAreaSize; rdz <= loadAreaSize; rdz++)
				{
					int chunkX = x + rdx;
					int chunkZ = z + rdz;
					if (auto c = GetChunk(chunkX * 16, chunkZ * 16); c)
					{
						c->AliveTime = 0;
					}
					else
					{
						auto chunk = std::make_shared<Chunk>(this, chunkX, chunkZ);

						if (m_SaveManager.Load(chunk.get()))
						{
							m_ChunkMap[GetChunkCoord(chunkX, chunkZ)] = chunk;
						}
					}
				}
			}

			idx++;
			if (idx % 100 == 0)
			{
				char buffer[64];
				snprintf(buffer, 64, "Lightning %i/%i", idx, count);
				WorldAccess->RenderLoadingScreenWithBar(buffer, (float)idx / count);
			}

			auto chunk = GetChunk(x * 16, z * 16);
			auto random = Random(Seed);

			auto l1 = (random.NextInt() / 2) * 2 + 1;
			auto l2 = (random.NextInt() / 2) * 2 + 1;

			auto chunkSeed = chunk->PosX * l1 + chunk->PosZ * l2;

			random.SetSeed(chunkSeed);

			for (int rdx = -loadAreaSize; rdx <= loadAreaSize; rdx++)
			{
				for (int rdz = -loadAreaSize; rdz <= loadAreaSize; rdz++)
				{
					int chunkX = x + rdx;
					int chunkZ = z + rdz;
					if (auto c = GetChunk(chunkX * 16, chunkZ * 16); c)
					{
						if (c->GenStage == TERRAIN_GEN_LIGHTING)
						{
							for (int x = 0; x < 16; x++)
							{
								for (int z = 0; z < 16; z++)
								{
									c->ComputeColumnHeight(x, z);
									for (int y = 0; y < CHUNK_HEIGHT; y++)
									{
										c->SetSkyLightLocal(x, y, z, c->GetHeightLocal(x, z) <= y ? 15 : 0);
									}
								}
							}
							c->GenStage = TERRAIN_GEN_LIGHTING3;
						}
					}
				}
			}

			if (chunk->GenStage == TERRAIN_GEN_LIGHTING3)
			{
				CachedChunkAccess access(this, chunk->GetStartX(), chunk->GetStartZ(), 1);

				for (int x = 0; x < 16; x++)
				{
					for (int z = 0; z < 16; z++)
					{
						int xx = chunk->GetStartX() + x;
						int zz = chunk->GetStartZ() + z;

						UpdateSkyLightColumn(xx, zz, &access);

						for (int y = 0; y < CHUNK_HEIGHT; y++)
						{
							if (chunk->GetBlockLightLocal(x, y, z) < chunk->GetBlock(x, y, z).GetDef()->GetLightEmitted())
							{
								UpdateBlockLight(chunk->GetStartX() + x, y, chunk->GetStartZ() + z, &access);
							}
						}
					}
				}

				chunk->GenStage = TERRAIN_GEN_COMPLETE;
			}

			for (auto it = m_ChunkMap.begin(); it != m_ChunkMap.end(); )
			{
				it->second->AliveTime++;

				if (it->second->AliveTime > 10)
				{
					auto chunk = it->second;

					if (m_JobQueue.size() < 32 && WORLD_SIZE >= 27)
					{
						PushTask([chunk, this]
							{
								m_SaveManager.Save(chunk.get());
							}, IsServerWorld);
					}
					else
					{
						m_SaveManager.Save(chunk.get());
					}

					it = m_ChunkMap.erase(it);
				}
				else
				{
					++it;
				}
			}

			zPos += 1;
		}
		xPos += 1;
	}

	count = m_JobQueue.size();
	while (!m_JobQueue.empty())
	{
		WorldAccess->RenderLoadingScreenWithBar("Saving", (float)(count - m_JobQueue.size()) / count);
	}

	m_ChunkMap.clear();
}

void World::SearchValidSpawn()
{
	WorldAccess->RenderLoadingScreen("Preparing World Generation");
	Random random{ Seed };
	int attempts = 0;
	while (1)
	{
		attempts++;
		SpawnX += random.NextInt(32) - 16;
		SpawnZ += random.NextInt(32) - 16;

		char buffer[64];
		snprintf(buffer, 64, "Looking for spawn area X: %i Z: %i", SpawnX, SpawnZ);
		WorldAccess->RenderLoadingScreen(buffer);

		RequestChunksSurroundingArea(SpawnX, SpawnZ, 0, true);

		int Height = GetHeight(SpawnX, SpawnZ);
		if (Height > 63)
		{
			if ((GetBlock(SpawnX, Height - 1, SpawnZ).Id == BLOCK_SAND) &&
				GetBlock(SpawnX, Height, SpawnZ).Id == BLOCK_AIR)
			{
				SpawnY = Height;
				break;
			}
			else
			{
				//printf("Expected 10 got %i\n", GetBlock(SpawnX, Height - 1, SpawnZ).Id);
			}
		}

		m_ChunkMap.clear();
	}
	m_ChunkMap.clear();
	printf("Found valid spawn at %i %i %i in %i attempts\n", SpawnX, SpawnY, SpawnZ, attempts);
}

void World::RequestChunksSurroundingArea(int x, int z, int loadDistance, bool locking, std::function<void(float)> cb)
{
	GetChunkProvider();

	int targetChunkCount = (loadDistance + 1) * 2 + 1;
	targetChunkCount *= targetChunkCount;

	int centerX = x / 16;
	int centerZ = z / 16;

	int rd = loadDistance + 1;
	int count = 0;

	for (int rx = -rd; rx < rd; rx++)
	{
		for (int rz = -rd; rz < rd; rz++)
		{
			int chunkX = centerX + rx;
			int chunkZ = centerZ + rz;

			auto chunk = GetChunk(chunkX * CHUNK_WIDTH, chunkZ * CHUNK_WIDTH);

			if (chunk)
			{
				chunk->AliveTime = 0;

				if (chunk->GenStage != TERRAIN_GEN_COMPLETE && !chunk->InWorldGenQueue)
				{
					chunk->InWorldGenQueue = true;
					m_ChunkGenQueue.push(m_ChunkMap[GetChunkCoord(chunkX, chunkZ)]);
				}
			}
			else
			{
				auto chunkref = std::make_shared<Chunk>(this, chunkX, chunkZ);
				chunkref->InWorldGenQueue = true;
				chunkref->GenStage = TERRAIN_GEN_UNPOPULATED;

				auto loadFn = [chunkref, this]
					{
						if (!m_SaveManager.Load(chunkref.get()))
						{
							Random random(Seed);
							m_ChunkProvider->ProvideShape(*chunkref, random, Seed);
						}
						else if (WorldAccess)
						{
							WorldAccess->SetColumnDirty(chunkref->GetStartX(), chunkref->GetStartZ());
							WorldAccess->SetColumnDirty(chunkref->GetStartX() - 16, chunkref->GetStartZ());
							WorldAccess->SetColumnDirty(chunkref->GetStartX() + 16, chunkref->GetStartZ());
							WorldAccess->SetColumnDirty(chunkref->GetStartX(), chunkref->GetStartZ() - 16);
							WorldAccess->SetColumnDirty(chunkref->GetStartX(), chunkref->GetStartZ() + 16);
							WorldAccess->SetColumnDirty(chunkref->GetStartX() - 16, chunkref->GetStartZ() - 16);
							WorldAccess->SetColumnDirty(chunkref->GetStartX() + 16, chunkref->GetStartZ() + 16);
							WorldAccess->SetColumnDirty(chunkref->GetStartX() - 16, chunkref->GetStartZ() + 16);
							WorldAccess->SetColumnDirty(chunkref->GetStartX() + 16, chunkref->GetStartZ() - 16);
						}
					};

				if (!locking)
				{
					PushTask(loadFn, IsServerWorld);
				}
				else
				{
					loadFn();
					count++;
					if (cb && count % 10 == 0)
						cb((float)count / targetChunkCount);
				}

				if (!locking || cb)
				{
					m_ChunkGenQueue.push(chunkref);
				}
				else
				{
					for (int x = 0; x < 16; x++)
					{
						for (int z = 0; z < 16; z++)
						{
							chunkref->ComputeColumnHeight(x, z);
						}
					}
				}
				m_ChunkMap[GetChunkCoord(chunkX, chunkZ)] = chunkref;
				chunk = chunkref.get();
				if (WorldAccess)
					WorldAccess->SetColumnDirty(chunkref->GetStartX(), chunkref->GetStartZ());
			}
		}
	}
}

void World::ProcessWorldGenQueue()
{
	for (int i = 0; i < 8; i++)
	{
		if (!m_ChunkGenQueue.empty())
		{
			auto chunk = m_ChunkGenQueue.front();
			m_ChunkGenQueue.pop();

			if (chunk->AliveTime > 390)
			{
				continue;
			}

			auto random = Random(Seed);

			auto l1 = (random.NextInt() / 2) * 2 + 1;
			auto l2 = (random.NextInt() / 2) * 2 + 1;

			auto chunkSeed = chunk->PosX * l1 + chunk->PosZ * l2;

			random.SetSeed(chunkSeed);

			// Surface -> Features
			if (chunk->GenStage == TERRAIN_GEN_SURFACE && chunk->HasAllNeighboursLoaded())
			{
				chunk->GenStage = TERRAIN_GEN_FEATURES;
				PushTask([this, chunk, chunkSeed]
					{
						for (int x = 0; x < 16; x++)
						{
							for (int z = 0; z < 16; z++)
							{
								chunk->ComputeColumnHeight(x, z);
							}
						}

						auto random = Random(chunkSeed);
						m_ChunkProvider->DecorateSurface(*chunk, random, Seed);
						chunk->InWorldGenQueue = false;

						chunk->GenStage = TERRAIN_GEN_LIGHTING;
					}, IsServerWorld);
			}
			else if (chunk->GenStage == TERRAIN_GEN_FEATURES && chunk->HasAllNeighboursLoaded())
			{
				chunk->GenStage = TERRAIN_GEN_LIGHTING;
			}
			else if (chunk->GenStage != TERRAIN_GEN_COMPLETE)
			{
				m_ChunkGenQueue.push(chunk);
			}
		}
	}
}

Chunk* World::GetChunk(ChunkCoord coord)
{
	if (auto chunk = m_ChunkMap.find(coord); chunk != m_ChunkMap.end())
		return chunk->second.get();
	return NULL;
}

Chunk* World::GetChunk(int x, int z)
{
	x = x >> 4;
	z = z >> 4;
	int64_t l = x;
	int64_t l1 = z;
	auto coord = l & 0xffffffffL | (l1 & 0xffffffffL) << 32;
	if (auto chunk = m_ChunkMap.find(coord); chunk != m_ChunkMap.end())
		return chunk->second.get();
	return NULL;
}

bool World::AreChunksLoaded(int x, int z, int rd, TerrainGenerationStage stage)
{
	for (int xx = -rd; xx <= rd; xx++)
	{
		for (int zz = -rd; zz <= rd; zz++)
		{
			int x1 = x + xx;
			int z1 = z + zz;
			auto chunk = GetChunk(x1 * CHUNK_WIDTH, z1 * CHUNK_WIDTH);
			if (!chunk)
			{
				return false;
			}
			if (chunk->GenStage < stage)
			{
				return false;
			}
		}
	}

	return true;
}

void World::SetBlock(int x, int y, int z, BlockStorage block)
{
	SetBlockNoNotify(x, y, z, block);
	NotifyNeighboursWithDelay(x, y, z, 1, TICK_TYPE_NORMAL);
}

void World::SetBlockNoNotify(int x, int y, int z, BlockStorage block)
{
	SetBlockNoNotifyNoDirty(x, y, z, block);

	if (WorldAccess)
	{
		WorldAccess->SetBlockDirty(x, y, z);
		WorldAccess->SetBlockDirty(x, y - 1, z);
		WorldAccess->SetBlockDirty(x, y + 1, z);
		WorldAccess->SetBlockDirty(x - 1, y, z);
		WorldAccess->SetBlockDirty(x + 1, y, z);
		WorldAccess->SetBlockDirty(x, y, z - 1);
		WorldAccess->SetBlockDirty(x, y, z + 1);
	}

	auto chunk = GetChunk(x, z);

	if (chunk)
	{
		CachedChunkAccess access(this, x, z, 1);
		chunk->ComputeColumnHeight(x, z);
		if (HasSky())
		{
			UpdateSkyLightColumn(x, z, &access);
		}
		UpdateBlockLight(x, y, z, &access);
	}
}

void World::SetBlockNoNotifyNoDirty(int x, int y, int z, BlockStorage block)
{
	if (y < 0 || y >= CHUNK_HEIGHT)
		return;

	auto chunk = GetChunk(x, z);

	if (chunk)
	{
		chunk->SetBlock(x, y, z, block);
		if (WorldAccess && chunk->GenStage == TERRAIN_GEN_COMPLETE)
		{
			WorldAccess->OnBlockChanged(x, y, z);
		}
	}
}

void World::SetDirtyFlag(int x, int y, int z)
{
	if (WorldAccess)
	{
		WorldAccess->SetBlockDirty(x, y, z);
		WorldAccess->SetBlockDirty(x, y - 1, z);
		WorldAccess->SetBlockDirty(x, y + 1, z);
		WorldAccess->SetBlockDirty(x - 1, y, z);
		WorldAccess->SetBlockDirty(x + 1, y, z);
		WorldAccess->SetBlockDirty(x, y, z - 1);
		WorldAccess->SetBlockDirty(x, y, z + 1);
	}
}

void World::NotifyNeighboursWithDelay(int x, int y, int z, uint16_t delay, uint16_t type)
{
	ScheduleUpdate(x - 1, y, z, delay, type);
	ScheduleUpdate(x + 1, y, z, delay, type);
	ScheduleUpdate(x, y, z - 1, delay, type);
	ScheduleUpdate(x, y, z + 1, delay, type);
	ScheduleUpdate(x, y - 1, z, delay, type);
	ScheduleUpdate(x, y + 1, z, delay, type);
	ScheduleUpdate(x, y, z, delay, type);
}

void World::NotifyBlock(int x, int y, int z)
{
	m_TickList.emplace_back(x, y, z, 0, 0);
}

void World::ScheduleUpdate(int x, int y, int z, uint16_t delay, uint16_t type)
{
	if (delay == 0xFFFF)
	{
		GetBlock(x, y, z).GetDef()->Tick(this, x, y, z);
		return;
	}
	auto chunk = GetChunk(x, z);
	if (!chunk || chunk->GenStage != TERRAIN_GEN_COMPLETE)
	{
		return;
	}
	m_TickList.emplace_back(x, y, z, delay, type);
}

void World::UpdateSkyLightColumn(int x, int z, CachedChunkAccess* access)
{
	if (!HasSky())
		return;
	if (access)
	{
		int columnHeight = access->GetHeight(x, z);
		int level = 0;

		for (int y = CHUNK_HEIGHT - 1; y >= 0; y--)
		{
			auto b = access->GetBlock(x, y, z).GetDef();

			level = y >= access->GetHeight(x, z) ? 15 : 0;

			if (level < 0)
				level = 0;

			bool update = false;
			if ((b->GetOpacity() != 0 && access->GetSkyLight(x, y, z) != 0) || !b->IsOpaque() || !b->BlocksLightToTheSides()) {
				update = true;
			}

			if (update)
			{
				UpdateSkyLight(x, y, z, access);
			}
			else
			{
				access->SetSkyLight(x, y, z, level);
			}
		}
	}
}

bool World::IsOutsideWorld(int x, int z)
{
#ifdef LIMITED_WORLD
	const int worldSize = WORLD_SIZE * 16;
	return x <= -worldSize + SpawnX || x >= worldSize + SpawnX || z <= -worldSize + SpawnZ || z >= worldSize + SpawnZ;
#else
	return false;
#endif
}

BlockStorage World::GetBlock(int x, int y, int z)
{
	if (y < 0 || y >= CHUNK_HEIGHT)
		return BLOCK_AIR;

	auto chunk = GetChunk(x, z);
	
	if (chunk)
	{
		return chunk->GetBlock(x, y, z);
	}

	return BLOCK_STONE;
}

int World::GetHeight(int x, int z)
{
	auto chunk = GetChunk(x, z);

	if (chunk)
	{
		return chunk->GetHeightLocal(x, z);
	}

	return CHUNK_HEIGHT - 1;
}

void World::SetSkyLight(int x, int y, int z, int level)
{
	auto chunk = GetChunk(x, z);
	if (chunk)
	{
		chunk->SetSkyLight(x, y, z, level);
	}
}

int World::GetSkyLight(int x, int y, int z)
{
	auto chunk = GetChunk(x, z);

	if (chunk)
	{
		return chunk->GetSkyLight(x, y, z);
	}

	return 15;
}

int World::GetSkyLightDecreased(int x, int y, int z)
{
	return GetSkyLight(x, y, z) - m_InternalSkylightDecrease;
}

void World::SetBlockLight(int x, int y, int z, int level)
{
	auto chunk = GetChunk(x, z);

	if (chunk)
	{
		return chunk->SetBlockLight(x, y, z, level);
	}
}

int World::GetBlockLight(int x, int y, int z)
{
	auto chunk = GetChunk(x, z);

	if (chunk)
	{
		return chunk->GetBlockLight(x, y, z);
	}

	return 0;
}

int World::GetMixedLight(int x, int y, int z)
{
	return glm::max(GetBlockLight(x, y, z), GetSkyLight(x, y, z) - m_InternalSkylightDecrease);
}

int World::GetMixedLightNoDecrease(int x, int y, int z)
{
	return glm::max(GetBlockLight(x, y, z), GetSkyLight(x, y, z));
}

void World::SetTileEntity(int x, int y, int z, std::shared_ptr<TileEntity> entity)
{
	auto chunk = GetChunk(x, z);

	if (chunk)
	{
		chunk->SetTileEntity(x, y, z, entity);
	}
}

TileEntity* World::GetTileEntity(int x, int y, int z)
{
	auto chunk = GetChunk(x, z);

	if (chunk)
	{
		return chunk->GetTileEntity(x, y, z);
	}

	return nullptr;
}

void World::GetBoxes(AABB aabb, AABB* results, int& count)
{
	int maxCount = count;
	count = 0;

	const float EPSILON = 1.0f;

	float x0 = aabb.minX - EPSILON;
	float y0 = aabb.minY - EPSILON;
	float z0 = aabb.minZ - EPSILON;
	float x1 = aabb.maxX + EPSILON;
	float y1 = aabb.maxY + EPSILON;
	float z1 = aabb.maxZ + EPSILON;

	for (float x = x0; x < x1; x += 1.0) {
		for (float y = y0; y < y1; y += 1.0) {
			for (float z = z0; z < z1; z += 1.0) {

				if (count > maxCount - 1)
					return;

				int fx = (int)glm::floor(x);
				int fy = (int)glm::floor(y);
				int fz = (int)glm::floor(z);

				if (IsOutsideWorld(fx, fz))
				{
					AABB bbs[4]{};
					int boxesCount = Block::GetBlock(1)->GetCollisionBoundingBoxes(this, fx, fy, fz, bbs);;
					for (int i = 0; i < boxesCount; i++)
					{
						if (count + 1 < maxCount - 1)
						{
							results[count] = bbs[i];
							count++;
						}
					}
					continue;
				}

				BlockStorage block = GetBlock(fx, fy, fz);

				if (Block::GetBlock(block.Id)->IsSolid()) {
					AABB bbs[4]{};
					int boxesCount = block.GetDef()->GetCollisionBoundingBoxes(this, fx, fy, fz, bbs);
					for (int i = 0; i < boxesCount; i++)
					{
						if (count + 1 < maxCount - 1)
						{
							results[count] = bbs[i];
							count++;
						}
					}
				}

			}
		}
	}
}

BlockLiquidType World::IsBoundingBoxInLiquid(AABB aabb)
{
	const float EPSILON = 1.0f;
	float x0 = aabb.minX - EPSILON;
	float y0 = aabb.minY - EPSILON;
	float z0 = aabb.minZ - EPSILON;
	float x1 = aabb.maxX + EPSILON;
	float y1 = aabb.maxY + EPSILON;
	float z1 = aabb.maxZ + EPSILON;

	for (float x = x0; x < x1; x += 1.0) {
		for (float y = y0; y < y1; y += 1.0) {
			for (float z = z0; z < z1; z += 1.0) {
				int fx = (int)glm::floor(x);
				int fy = (int)glm::floor(y);
				int fz = (int)glm::floor(z);

				BlockStorage block = GetBlock(fx, fy, fz);

				if (block.GetDef()->GetLiquidType() != 0) {

					AABB bb(0.0f, 0.0f, 0.0f, 1.0f, 0.9f, 1.0f);
					bb.move(fx, fy, fz);

					if (bb.intersects(aabb))
					{
						return (BlockLiquidType)block.GetDef()->GetLiquidType();
					}
				}

			}
		}
	}

	return LIQUID_TYPE_NONE;
}

bool World::IsBoundingBoxFree(AABB box)
{
	const float EPSILON = 1.0f;
	float x0 = box.minX - EPSILON;
	float y0 = box.minY - EPSILON;
	float z0 = box.minZ - EPSILON;
	float x1 = box.maxX + EPSILON;
	float y1 = box.maxY + EPSILON;
	float z1 = box.maxZ + EPSILON;

	for (float x = x0; x < x1; x += 1.0) {
		for (float y = y0; y < y1; y += 1.0) {
			for (float z = z0; z < z1; z += 1.0) {
				int fx = (int)glm::floor(x);
				int fy = (int)glm::floor(y);
				int fz = (int)glm::floor(z);

				BlockStorage block = GetBlock(fx, fy, fz);

				if (Block::GetBlock(block.Id)->IsSolid()) {
					AABB bbs[4]{};
					int boxesCount = block.GetDef()->GetCollisionBoundingBoxes(this, fx, fy, fz, bbs);
					for (int i = 0; i < boxesCount; i++)
					{
						if (bbs[i].intersects(box))
						{
							return false;
						}
					}
				}

			}
		}
	}

	return true;
}

// I could return a new vector everytime i call this
// But i need to reduce memory allocations by all means possible
BaseEntity** World::GetCollidingEntitiesWith(AABB bb, int* count)
{
	assert(count);
	m_CollidingEntities.clear();
	for (int i = 0; i < m_Entities.size(); i++)
	{
		if (m_Entities[i]->BoundingBox.intersects(bb))
		{
			m_CollidingEntities.push_back(m_Entities[i].get());
		}
	}
	*count = static_cast<int>(m_CollidingEntities.size());
	return m_CollidingEntities.data();
}

BaseEntity* World::GetEntityWithRayCast(BaseEntity* src, const glm::vec3& org, const glm::vec3& dir, float maxDistance)
{
	return GetEntityWithRayCast(src, nullptr, org, dir, maxDistance);
}

BaseEntity* World::GetEntityWithRayCast(BaseEntity* src, BaseEntity* dst, const glm::vec3& org, const glm::vec3& dir, float maxDistance)
{
	BaseEntity* closestEntity = nullptr;
	float closestDistance = maxDistance;

	float rayDist = maxDistance;

	if (dst)
	{
		rayDist = glm::distance(org, dst->Position);
	}

	for (int i = 0; i < m_Entities.size(); i++)
	{
		if (dst && m_Entities[i].get() != dst)
			continue;
		if (m_Entities[i].get() != src && m_Entities[i]->BoundingBox.intersectsRay(org, dir))
		{
			float dist = glm::distance(org, m_Entities[i]->Position);
			if (dist <= closestDistance && m_Entities[i]->CanBeRaycasted)
			{
				closestDistance = dist;
				closestEntity = m_Entities[i].get();
			}
		}
	}

	rayDist = closestDistance;
	float stepSize = 0.1f;
	auto steps = (int)(rayDist / stepSize);
	auto currentPos = org;
	auto stepDir = glm::normalize(dir) * stepSize;

	for (int i = 0; i < steps && closestEntity; i++)
	{
		currentPos += stepDir;

		auto rayBB = AABB(currentPos.x - stepSize, currentPos.y - stepSize, currentPos.z - stepSize, currentPos.x + stepSize, currentPos.y + stepSize, currentPos.z + stepSize);

		AABB boxes[32];
		int boxCount = 32;
		GetBoxes(rayBB, boxes, boxCount);

		for (int j = 0; j < boxCount; j++)
		{
			glm::vec3 minCorner(boxes[j].minX, boxes[j].minY, boxes[j].minZ);
			glm::vec3 maxCorner(boxes[j].maxX, boxes[j].maxY, boxes[j].maxZ);

			glm::vec3 closestPointToOrigin = glm::clamp(org, minCorner, maxCorner);
			float dist = glm::distance(closestPointToOrigin, org);
			if (dist < rayDist && boxes[j].intersects(rayBB))
			{
				return nullptr;
			}
		}
	}

	return closestEntity;
}

BaseEntity* World::GetClosestPlayer(float x, float z, float distance)
{
	BaseEntity* closestEntity = NULL;

	for (int i = 0; i < m_Entities.size(); i++)
	{
		if (dynamic_cast<PlayerEntity*>(m_Entities[i].get()))
		{
			auto entity = m_Entities[i].get();
			float d = glm::distance(glm::vec2(entity->Position.x, entity->Position.z), glm::vec2(x, z));
			if (d < distance)
			{
				closestEntity = entity;
				distance = d;
			}
		}
	}

	return closestEntity;
}

BaseEntity* World::GetEntityByUUID(uint64_t uuid)
{
	if (uuid == 0)
		return NULL;
	for (int i = 0; i < m_Entities.size(); i++)
	{
		if (m_Entities[i]->GetUUID() == uuid)
		{
			return m_Entities[i].get();
		}
	}
	return nullptr;
}

bool World::RayCast(const glm::vec3& org, const glm::vec3& dir, float maxDistance, RayCastHit& out)
{
	const float reach = maxDistance;
	const float mult = 1.0f / 128.0f;

	float xp = org.x;
	float yp = org.y;
	float zp = org.z;

	float xp0 = xp;
	float yp0 = yp;
	float zp0 = zp;

	glm::vec3 forward = dir;

	float lenx = forward.x * mult;
	float leny = forward.y * mult;
	float lenz = forward.z * mult;

	float distance = 0;

	while (distance < reach) {
		BlockStorage tileAt = GetBlock((int)floor(xp), (int)floor(yp), (int)floor(zp));
		if (IsOutsideWorld((int)floor(xp), (int)floor(zp)))
		{
			return false;
		}
		AABB pointBB = AABB(xp - mult / 2.0f, yp - mult / 2.0f, zp - mult / 2.0f, xp + mult / 2.0f, yp + mult / 2.0f, zp + mult / 2.0f);

		bool intersects = false;
		AABB bb = tileAt.GetDef()->GetRaycastBoundingBox(this, (int)floor(xp), (int)floor(yp), (int)floor(zp));
		intersects = bb.intersects(pointBB);

		if (tileAt.Id != 0 && tileAt.GetDef()->IsSolidToRaycast() && intersects) {

			int f = -1;

			float x = xp - xp0;
			float y = yp - yp0;
			float z = zp - zp0;

			if (y > 0.0 && f == -1) {
				f = 0;
			}
			if (y < 0.0 && f == -1) {
				f = 1;
			}
			if (z > 0.0 && f == -1) {
				f = 2;
			}
			if (z < 0.0 && f == -1) {
				f = 3;
			}
			if (x > 0.0 && f == -1) {
				f = 4;
			}
			if (x < 0.0 && f == -1) {
				f = 5;
			}

			if (f == -1)
				f = 6;

			glm::ivec4 fresult((int)floor(xp), (int)floor(yp), (int)floor(zp), f);
			out.Face = f;
			out.HitBlock = fresult;
			out.HitPosition = { xp, yp, zp };
			return true;
		}

		xp0 = xp;
		yp0 = yp;
		zp0 = zp;

		if (lenx < leny) {
			if (lenx < lenz) {
				lenx = (distance = lenx + mult);
				xp += forward.x * mult;
			}
			else {
				lenz = (distance = lenz + mult);
				zp += forward.z * mult;
			}
		}
		else if (leny < lenz) {
			leny = (distance = leny + mult);
			yp += forward.y * mult;
		}
		else {
			lenz = (distance = lenz + mult);
			zp += forward.z * mult;
		}

		distance = glm::distance(org, { (float)xp, (float)yp, (float)zp });
	}

	out.Face = -1;

	return false;
}

bool World::RayCastLiquids(const glm::vec3& org, const glm::vec3& dir, float maxDistance, RayCastHit& out)
{
	const float reach = maxDistance;
	const float mult = 1.0f / 128.0f;

	float xp = org.x;
	float yp = org.y;
	float zp = org.z;

	float xp0 = xp;
	float yp0 = yp;
	float zp0 = zp;

	glm::vec3 forward = dir;

	float lenx = forward.x * mult;
	float leny = forward.y * mult;
	float lenz = forward.z * mult;

	float distance = 0;

	while (distance < reach) {
		BlockStorage tileAt = GetBlock((int)floor(xp), (int)floor(yp), (int)floor(zp));
		if (IsOutsideWorld((int)floor(xp), (int)floor(zp)))
		{
			return false;
		}
		AABB pointBB = AABB(xp - mult / 2.0f, yp - mult / 2.0f, zp - mult / 2.0f, xp + mult / 2.0f, yp + mult / 2.0f, zp + mult / 2.0f);

		bool intersects = false;
		AABB bb = AABB(0.0f, 0.0f, 0.0f, 1.0f, 0.9f, 1.0f);
		bb.move(floor(xp), floor(yp), floor(zp));

		intersects = bb.intersects(pointBB);

		if (tileAt.Id != 0 && tileAt.GetDef()->GetLiquidType() != 0 && intersects && tileAt.Metadata == 8) {

			int f = -1;

			float x = xp - xp0;
			float y = yp - yp0;
			float z = zp - zp0;

			if (y > 0.0 && f == -1) {
				f = 0;
			}
			if (y < 0.0 && f == -1) {
				f = 1;
			}
			if (z > 0.0 && f == -1) {
				f = 2;
			}
			if (z < 0.0 && f == -1) {
				f = 3;
			}
			if (x > 0.0 && f == -1) {
				f = 4;
			}
			if (x < 0.0 && f == -1) {
				f = 5;
			}

			if (f == -1)
				f = 6;

			glm::ivec4 fresult((int)floor(xp), (int)floor(yp), (int)floor(zp), f);
			out.HitBlock = fresult;
			out.HitPosition = { xp, yp, zp };
			return true;
		}

		xp0 = xp;
		yp0 = yp;
		zp0 = zp;

		if (lenx < leny) {
			if (lenx < lenz) {
				lenx = (distance = lenx + mult);
				xp += forward.x * mult;
			}
			else {
				lenz = (distance = lenz + mult);
				zp += forward.z * mult;
			}
		}
		else if (leny < lenz) {
			leny = (distance = leny + mult);
			yp += forward.y * mult;
		}
		else {
			lenz = (distance = lenz + mult);
			zp += forward.z * mult;
		}

		distance = glm::distance(org, { (float)xp, (float)yp, (float)zp });
	}
	return false;
}

void World::RequestPathForEntity(BaseEntity* entity, const glm::ivec3& start, const glm::ivec3& goal)
{
	if (IsOutsideWorld(goal.x, goal.z))
	{
		return;
	}
	PathFinder finder(this);
	auto path = finder.FindPath(start, goal);
	entity->OnPathReady(path);
}

float World::GetBoundingBoxVisibilityFromPoint(const glm::vec3& point, AABB bb)
{
	float stepsX = 1.0f / ((bb.maxX - bb.minX) * 2.0f + 1.0f);
	float stepsY = 1.0f / ((bb.maxY - bb.minY) * 2.0f + 1.0f);
	float stepsZ = 1.0f / ((bb.maxZ - bb.minZ) * 2.0f + 1.0f);
	int visiblePoints = 0;
	int samplePoints = 0;

	for (float sx = 0.0F; sx <= 1.0F; sx += stepsX) {
		for (float sy = 0.0F; sy <= 1.0F; sy += stepsY) {
			for (float sz = 0.0F; sz <= 1.0F; sz += stepsZ) {
				float px = bb.minX + (bb.maxX - bb.minX) * (float)sx;
				float py = bb.minY + (bb.maxY - bb.minY) * (float)sy;
				float pz = bb.minZ + (bb.maxZ - bb.minZ) * (float)sz;
				RayCastHit hit;
				auto dir = glm::vec3(px, py, pz);
				if (!RayCast(point, glm::normalize(dir), glm::length(point - dir), hit)) {
					++visiblePoints;
				}

				++samplePoints;
			}
		}
	}

	return (float)visiblePoints / (float)samplePoints;
}

void World::DoExplosion(BaseEntity* owner, float x, float y, float z, float power)
{
	auto uuid = 0ULL;
	if (owner)
	{
		uuid = owner->GetUUID();
	}

	Explosion ex = Explosion(this, uuid, x, y, z, power);
	ex.CalculateBlocks();
	ex.ExplodeBlocks();
}

AABB World::GetBlockBoundingBox(int x, int y, int z)
{
	if (auto block = GetBlock(x, y, z); block.Id != 0)
	{
		return block.GetDef()->GetRaycastBoundingBox(this, x, y, z);
	}
	return {};
}

void World::AddEntity(std::shared_ptr<BaseEntity> entity)
{
	if (!entity || (m_Entities.size() > ENTITY_CAP && !entity->IsImportant))
	{
		return;
	}

	entity->Dimension = GetDimensionID();

	m_Entities.push_back(entity);
	if (WorldAccess)
		WorldAccess->OnEntitySpawned(entity->GetUUID());
}

bool World::CanAddEntity()
{
	return m_Entities.size() + 1 < ENTITY_CAP;
}

void World::Update()
{
	Random rng;

	MobSpawner::PerformMobSpawning(this);
	UpdateInternalSkylightDecrease(1.0f);

	m_Ticks++;

	m_AliveEntities.clear();
	for (int i = 0; i < m_Entities.size(); i++)
	{
		auto entity = m_Entities[i];
		if (!entity->IsDead || !entity->IsRemovable)
			entity->Update();
		if ((!entity->IsDead || !entity->IsRemovable) && 
			!entity->ForceRemove &&
			entity->Dimension == GetDimensionID())
		{
			m_AliveEntities.push_back(entity);
		}
		else
		{
			if (WorldAccess)
				WorldAccess->OnEntityDestroy(entity->GetUUID());
		}
	}
	m_Entities.clear();
	for (int i = 0; i < m_AliveEntities.size(); i++)
	{
		m_Entities.push_back(m_AliveEntities[i]);
	}

	if (!m_Entities.empty())
	{
		// Dirty cheat to get the player, TO DO: Add a function to set the player render area position
		auto player = m_Entities[0].get();

		int renderDistance = LOAD_DISTANCE;
#ifdef __wii__
		renderDistance = LOAD_DISTANCE;
#endif

		RequestChunksSurroundingArea(floor(player->Position.x), floor(player->Position.z), renderDistance + 1);
	}

	uint64_t toUnload[4]{};
	int amountToUnload = false;

	ProcessWorldGenQueue();

	int doLightUpdate = 128;
	int maxSavesPerTick = 4;

	for (auto& kv : m_ChunkMap)
	{
		auto chunk = kv.second.get();
		chunk->AliveTime++;

		if (chunk->AliveTime > 400 && amountToUnload < 4)
		{
			toUnload[amountToUnload++] = GetChunkCoord(chunk->PosX, chunk->PosZ);
		}

		const int updates = 3;
		const int sections = CHUNK_HEIGHT / 16;

		for (int j = 0; j < sections && chunk->GenStage == TERRAIN_GEN_COMPLETE; j++)
		{
			for (int i = 0; i < updates; i++) {
				int x = rng.NextInt(CHUNK_WIDTH) + chunk->GetStartX();
				int y = rng.NextInt(16) + j * 16;
				int z = rng.NextInt(CHUNK_WIDTH) + chunk->GetStartZ();

				Block* block = chunk->GetBlockGlobal(x, y, z).GetDef();
				if (block) {
					block->RandomTick(this, x, y, z);
				}
			}
		}

		if (chunk->SaveTicks > 0)
			chunk->SaveTicks--;

		if (chunk->GenStage >= TERRAIN_GEN_SURFACE && chunk->SaveTicks == 1 && maxSavesPerTick > 0)
		{
			maxSavesPerTick--;
			chunk->SaveTicks = 0;
			auto ref = kv.second;
			PushTask([ref, this]
			{
				m_SaveManager.Save(ref.get());
			}, IsServerWorld);
		}

		if (chunk->GenStage != TERRAIN_GEN_COMPLETE)
		{
			if (chunk->GenStage == TERRAIN_GEN_LIGHTING && AreChunksLoaded(chunk->PosX, chunk->PosZ, 1, TERRAIN_GEN_FEATURES))
			{
				chunk->GenStage = TERRAIN_GEN_LIGHTING2;
				PushTask([this, chunk]
					{
						m_ChunkProvider->LightChunk(*chunk);
						chunk->GenStage = TERRAIN_GEN_LIGHTING3;
					}, IsServerWorld);
			}
			if (chunk->GenStage == TERRAIN_GEN_LIGHTING3 && AreChunksLoaded(chunk->PosX, chunk->PosZ, 1, TERRAIN_GEN_LIGHTING3))
			{
				chunk->GenStage = TERRAIN_GEN_LIGHTING4;
				PushTask([this, chunk] {
					m_ChunkProvider->PropogateChunk(*chunk);

					if (WorldAccess)
					{
						WorldAccess->SetColumnDirty(chunk->GetStartX(), chunk->GetStartZ());
						WorldAccess->SetColumnDirty(chunk->GetStartX() - 16, chunk->GetStartZ());
						WorldAccess->SetColumnDirty(chunk->GetStartX() + 16, chunk->GetStartZ());
						WorldAccess->SetColumnDirty(chunk->GetStartX(), chunk->GetStartZ() - 16);
						WorldAccess->SetColumnDirty(chunk->GetStartX(), chunk->GetStartZ() + 16);
						WorldAccess->SetColumnDirty(chunk->GetStartX() - 16, chunk->GetStartZ() - 16);
						WorldAccess->SetColumnDirty(chunk->GetStartX() + 16, chunk->GetStartZ() + 16);
						WorldAccess->SetColumnDirty(chunk->GetStartX() - 16, chunk->GetStartZ() + 16);
						WorldAccess->SetColumnDirty(chunk->GetStartX() + 16, chunk->GetStartZ() - 16);
					}

					chunk->GenStage = TERRAIN_GEN_COMPLETE;
					}, IsServerWorld);
			}
		}

		if (chunk->GenStage == TERRAIN_GEN_COMPLETE)
		{
			auto ref = chunk->TileEntities;
			for (auto& tkv : ref)
			{
				tkv.second->Update();
			}
		}
	}

	for (int i = 0; i < amountToUnload; i++)
	{
		auto chunk = m_ChunkMap[toUnload[i]];

		if (chunk->GenStage >= TERRAIN_GEN_SURFACE)
		{
			PushTask([chunk, this]
			{
				m_SaveManager.Save(chunk.get());
			}, IsServerWorld);
		}

		m_ChunkMap.erase(toUnload[i]);

		if (WorldAccess)
		{
			WorldAccess->SetColumnDirty(chunk->GetStartX(), chunk->GetStartZ());
		}
	}

	int tickedThisTick = 0;
	const int maxTickedBlocks = 128;
	while (!m_TickList.empty())
	{
		auto tick = m_TickList.back();
		m_TickList.pop_back();

		tickedThisTick++;
#ifdef __wii__
		if (tickedThisTick > maxTickedBlocks)
			continue;
#endif

		if (tick.delay != 0)
		{
			tick.delay -= 1;
			m_DelayedTickList.push(tick);
			continue;
		}

		auto block = GetBlock(tick.x, tick.y, tick.z).GetDef();

		if (tick.type == TICK_TYPE_LIQUID)
		{
			block->FluidTick(this, tick.x, tick.y, tick.z);
			continue;
		}

		block->Tick(this, tick.x, tick.y, tick.z);
	}

	while (!m_DelayedTickList.empty())
	{
		m_TickList.push_back(m_DelayedTickList.front());
		m_DelayedTickList.pop();
	}

}

void World::NetworkUpdate()
{
	
}

std::vector<std::shared_ptr<BaseEntity>>& World::GetAllEntities()
{
	return m_Entities;
}

size_t World::GetEntityCount()
{
	return m_Entities.size();
}

void World::WriteToNbt(TagCompound& root)
{
	root.SetTag(std::make_shared<TagLong>("RandomSeed", Seed));
	root.SetInt("SpawnX", SpawnX);
	root.SetInt("SpawnY", SpawnY);
	root.SetInt("SpawnZ", SpawnZ);
	root.SetInt("WorldTime", m_Ticks);

	for (int i = 0; i < m_Entities.size(); i++)
	{
		auto player = dynamic_cast<PlayerEntity*>(m_Entities[i].get());
		if (player)
		{
			auto tag = std::make_shared<TagCompound>("Player");
			player->WriteToNbt(*tag);
			root.SetTag(tag);
		}
	}
}

void World::LoadFromNbt(TagCompound& root)
{
	Seed = root.GetTagAs<TagLong>("RandomSeed");
	SpawnX = root.GetInt("SpawnX");
	SpawnY = root.GetInt("SpawnY");
	SpawnZ = root.GetInt("SpawnZ");
	m_Ticks = root.GetInt("WorldTime");

	if (!m_Entities.empty())
	{
		auto player = static_cast<PlayerEntity*>(m_Entities[0].get());
		if (player)
		{
			auto& tag = root.GetTagAs<TagCompound>("Player");
			player->ReadFromNbt(tag);
		}
	}
}

float World::GetDayProgress()
{
	return (GetWorldTime() % 24000) / 24000.0f;
}

void World::UpdateInternalSkylightDecrease(float partialTicks)
{
	float angle = GetCelestialAngle(partialTicks);
	float light = 1.0f - (glm::cos(angle * glm::pi<float>() * 2.0f) * 2.0f + 0.5f);
	light = glm::clamp(light, 0.0f, 1.0f);
	int l = (int)(light * 11.0f);

	if (l != m_InternalSkylightDecrease)
	{
		m_InternalSkylightDecrease = l;
		if (WorldAccess)
		{
			WorldAccess->SetAllDirty();
		}
	}
}

float World::GetCelestialAngle(float partialTicks)
{
	int dayTime = GetWorldTime() % 24000;
	float time = (float)(dayTime + partialTicks) / 24000.0f - 0.25f;

	if (time < 0.0f)
	{
		time += 1.0f;
	}
	if (time > 1.0f)
	{
		time -= 1.0f;
	}

	float t = time;
	t = 1.0f - (glm::cos(t * glm::pi<float>()) + 1.0f) / 2.0f;
	t = time + (t - time) / 3.0f;

	return t;
}

int World::GetWorldTime()
{
	return m_Ticks;
}

void World::SetWorldTime(int time)
{
	m_Ticks = time;
}

std::vector<std::shared_ptr<BaseEntity>> World::GetEntitiesInChunk(Chunk* chunk)
{
	std::vector<std::shared_ptr<BaseEntity>> entities;

	AABB boundingBox = AABB{ (float)chunk->GetStartX(), 0.0f, (float)chunk->GetStartZ(),
		(float)chunk->GetStartX() + (float)CHUNK_WIDTH, (float)CHUNK_HEIGHT, (float)chunk->GetStartZ() + (float)CHUNK_WIDTH };

	for (int i = 0; i < m_Entities.size(); i++)
	{
		auto& entity = m_Entities[i];
		if (boundingBox.intersects(entity->Position.x, entity->Position.y, entity->Position.z))
		{
			entities.push_back(entity);
		}
	}

	return entities;
}

std::vector<BaseEntity*> World::GetPlayers()
{
	std::vector<BaseEntity*> players;

	for (int i = 0; i < m_Entities.size(); i++)
	{
		if (dynamic_cast<PlayerEntity*>(m_Entities[i].get()))
		{
			auto entity = m_Entities[i].get();
			players.push_back(entity);
		}
	}

	return players;
}

void World::Close()
{
	m_IsWorldActive = false;
	while (!m_IsWorldThreadRunning)
	{
		m_WakeCondition.notify_all();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

bool World::Load(std::function<void(float)> cb)
{
	if (m_SaveManager.Load(this))
	{
		//if (SpawnY == 0)
		//{
		//	SearchValidSpawn();
		//}
		if (!m_Entities.empty())
		{
			// Dirty cheat to get the player, TO DO: Add a function to set the player render area position
			auto player = m_Entities[0].get();

			int renderDistance = LOAD_DISTANCE;
#ifdef __wii__
			renderDistance = LOAD_DISTANCE;
#endif

			int targetChunkCount = (renderDistance + 1) * 2 + 1;
			targetChunkCount *= targetChunkCount;

			RequestChunksSurroundingArea((int)floor(player->Position.x), (int)floor(player->Position.z), renderDistance, true, cb);
		}
		return true;
	}

	return false;
}

void World::Save(std::function<void(float)> cb)
{
	m_SaveManager.Save(this);

	int count = 0;

	for (auto& kv : m_ChunkMap)
	{
		if (kv.second->GenStage >= TERRAIN_GEN_SURFACE)
		{
			m_SaveManager.Save(kv.second.get());
			count++;
			if (count % 10 == 0)
				cb((float)count / (float)m_ChunkMap.size());
		}
	}
}

SaveManager& World::GetSaveManager()
{
	return m_SaveManager;
}

ChunkProvider& World::GetChunkProvider()
{
	if (!m_ChunkProvider)
	{
		m_ChunkProvider = new ChunkProvider(Seed);
	}
	return *m_ChunkProvider;
}

Particle* World::SpawnParticle(std::string_view name, float x, float y, float z, TagCompound* extraData)
{
	if (WorldAccess)
	{
		return &WorldAccess->SpawnParticle(name, { x, y, z }, extraData);
	}
	return NULL;
}

void World::PlaySound(std::string_view name, float x, float y, float z, float volume)
{
	PlaySound(name, { x, y, z }, volume);
}

void World::PlaySound(std::string_view name, const glm::vec3& position, float volume)
{
	if (WorldAccess)
	{
		WorldAccess->PlayAudio(name, position, volume);
	}
}

void World::PlaySoundEffect(std::string_view name, float volume)
{
	if (WorldAccess)
	{
		WorldAccess->PlaySFX(name, volume);
	}
}

static inline int Encode(int x, int y, int z, int dx, int dy, int dz, int level)
{
	// dxyz - xyz + 32
	return dx - x + 32 + (dy - y + 32 << 6) + (dz - z + 32 << 12) + (level << 18);
}

static inline int DecodeX(int enc, int x)
{
	return (enc & 0x3F) - 32 + x;
}

static inline int DecodeY(int enc, int y)
{
	return (enc >> 6 & 0x3F) - 32 + y;
}

static inline int DecodeZ(int enc, int z)
{
	return (enc >> 12 & 0x3F) - 32 + z;
}

static inline int DecodeLight(int enc)
{
	return (enc >> 18) & 0xF;
}

void World::UpdateSkyLight(int x, int y, int z, CachedChunkAccess* access)
{
	int queue[16384];

	static const int faceOffsets[] =
	{
		0, -1, 0,
		0, 1, 0,
		-1, 0, 0,
		1, 0, 0,
		0, 0, -1,
		0, 0, 1
	};

	int tail = 0;
	int head = 0;

	int falloff = access->GetBlock(x, y, z).GetDef()->GetOpacity();
	int currentLightLevel = access->GetSkyLight(x, y, z);
	int computedLightLevel = ComputeSkyLevel(x, y, z, falloff, access);

	if (computedLightLevel == currentLightLevel)
		return;

	if (computedLightLevel > currentLightLevel)
	{
		queue[head++] = Encode(x, y, z, x, y, z, 0);
	}
	else
	{
		queue[head++] = Encode(x, y, z, x, y, z, currentLightLevel);
		while (tail < head)
		{
			int enc = queue[tail++];
			int xp = DecodeX(enc, x);
			int yp = DecodeY(enc, y);
			int zp = DecodeZ(enc, z);
			int level = DecodeLight(enc);
			int currentLevel = access->GetSkyLight(xp, yp, zp);
			if (level == currentLevel)
			{
				access->SetSkyLight(xp, yp, zp, 0);
				SetDirtyFlag(xp, yp, zp);

				if (level <= 0)
					continue;

				int taxiCabDistance = glm::abs(xp - x) + glm::abs(yp - y) + glm::abs(zp - z);
				if (taxiCabDistance >= 17)
					continue;

				for (int i = 0; i < 6; i++)
				{
					int xx = xp + faceOffsets[i * 3 + 0];
					int yy = yp + faceOffsets[i * 3 + 1];
					int zz = zp + faceOffsets[i * 3 + 2];
					int neighbourLevel = access->GetSkyLight(xx, yy, zz);
					int falloff = access->GetBlock(xx, yy, zz).GetDef()->GetOpacity();
					falloff = falloff < 1 ? 1 : falloff;
					if (neighbourLevel == level - falloff)
					{
						queue[head++] = Encode(x, y, z, xx, yy, zz, neighbourLevel);
					}
				}
			}
		}
		tail = 0;
	}

	while (tail < head)
	{
		int enc = queue[tail++];
		int xp = DecodeX(enc, x);
		int yp = DecodeY(enc, y);
		int zp = DecodeZ(enc, z);
		int currentLevel = access->GetSkyLight(xp, yp, zp);
		int falloff = access->GetBlock(xp, yp, zp).GetDef()->GetOpacity();
		int computedLevel = ComputeSkyLevel(xp, yp, zp, falloff, access);
		if (computedLevel != currentLevel)
		{
			access->SetSkyLight(xp, yp, zp, computedLevel);
			SetDirtyFlag(xp, yp, zp);

			int taxiCabDistance = glm::abs(xp - x) + glm::abs(yp - y) + glm::abs(zp - z);
			if (taxiCabDistance >= 17 || head >= 16384 - 6)
				continue;

			for (int i = 0; i < 6; i++)
			{
				int xx = xp + faceOffsets[i * 3 + 0];
				int yy = yp + faceOffsets[i * 3 + 1];
				int zz = zp + faceOffsets[i * 3 + 2];
				int neighbourLevel = access->GetSkyLight(xx, yy, zz);
				int falloff = access->GetBlock(xx, yy, zz).GetDef()->GetOpacity();

				if (neighbourLevel < computedLevel && falloff != 15)
				{
					queue[head++] = Encode(x, y, z, xx, yy, zz, 0);
				}
			}
		}
	}
}

void World::UpdateBlockLight(int x, int y, int z, CachedChunkAccess* access)
{
	std::array<int, 32768> queue;

	static const int faceOffsets[] =
	{
		0, -1, 0,
		0, 1, 0,
		-1, 0, 0,
		1, 0, 0,
		0, 0, -1,
		0, 0, 1
	};

	int tail = 0;
	int head = 0;

	int falloff = access->GetBlock(x, y, z).GetDef()->GetOpacity();
	int currentLightLevel = access->GetBlockLight(x, y, z);
	int computedLightLevel = ComputeBlockLevel(x, y, z, falloff, access);

	if (computedLightLevel == currentLightLevel)
		return;

	if (computedLightLevel > currentLightLevel)
	{
		queue[head++] = Encode(x, y, z, x, y, z, 0);
	}
	else
	{
		queue[head++] = Encode(x, y, z, x, y, z, currentLightLevel);
		while (tail < head)
		{
			int enc = queue[tail++];
			int xp = DecodeX(enc, x);
			int yp = DecodeY(enc, y);
			int zp = DecodeZ(enc, z);
			int level = DecodeLight(enc);
			int currentLevel = access->GetBlockLight(xp, yp, zp);
			if (level == currentLevel)
			{
				access->SetBlockLight(xp, yp, zp, 0);
				SetDirtyFlag(xp, yp, zp);

				if (level <= 0)
					continue;

				int taxiCabDistance = glm::abs(xp - x) + glm::abs(yp - y) + glm::abs(zp - z);
				if (taxiCabDistance >= (17 * 17))
					continue;

				for (int i = 0; i < 6; i++)
				{
					int xx = xp + faceOffsets[i * 3 + 0];
					int yy = yp + faceOffsets[i * 3 + 1];
					int zz = zp + faceOffsets[i * 3 + 2];
					int neighbourLevel = access->GetBlockLight(xx, yy, zz);
					int falloff = access->GetBlock(xx, yy, zz).GetDef()->GetOpacity();
					falloff = falloff < 1 ? 1 : falloff;
					if (neighbourLevel == level - falloff)
					{
						queue[head++] = Encode(x, y, z, xx, yy, zz, neighbourLevel);
					}
				}
			}
		}
		tail = 0;
	}

	while (tail < head)
	{
		int enc = queue[tail++];
		int xp = DecodeX(enc, x);
		int yp = DecodeY(enc, y);
		int zp = DecodeZ(enc, z);
		int currentLevel = access->GetBlockLight(xp, yp, zp);
		int falloff = access->GetBlock(xp, yp, zp).GetDef()->GetOpacity();
		int computedLevel = ComputeBlockLevel(xp, yp, zp, falloff, access);
		if (computedLevel != currentLevel)
		{
			access->SetBlockLight(xp, yp, zp, computedLevel);
			SetDirtyFlag(xp, yp, zp);

			int taxiCabDistance = glm::abs(xp - x) + glm::abs(yp - y) + glm::abs(zp - z);
			if (taxiCabDistance >= (17 * 17) || head >= queue.size() - 6)
				continue;

			for (int i = 0; i < 6; i++)
			{
				int xx = xp + faceOffsets[i * 3 + 0];
				int yy = yp + faceOffsets[i * 3 + 1];
				int zz = zp + faceOffsets[i * 3 + 2];
				int neighbourLevel = access->GetBlockLight(xx, yy, zz);
				int falloff = access->GetBlock(xx, yy, zz).GetDef()->GetOpacity();

				if (neighbourLevel < computedLevel && falloff != 15)
				{
					queue[head++] = Encode(x, y, z, xx, yy, zz, 0);
				}
			}
		}
	}
}

int World::ComputeSkyLevel(int x, int y, int z, int falloff, CachedChunkAccess* access)
{
	static constexpr int faceOffsets[] =
	{
		0, -1, 0,
		0, 1, 0,
		-1, 0, 0,
		1, 0, 0,
		0, 0, -1,
		0, 0, 1
	};

	int height = access->GetHeight(x, z);
	if (access->GetBlock(x, y, z).GetDef()->GetOpacity() >= 15)
		return 0;
	int computedLevel = 0;

	if (y >= height)
	{
		computedLevel = 15;
	}
	else
	{
		if (falloff == 0) {
			falloff = 1;
		}
		for (int i = 0; i < 6; i++) {
			int xx = x + faceOffsets[i * 3 + 0];
			int yy = y + faceOffsets[i * 3 + 1];
			int zz = z + faceOffsets[i * 3 + 2];

			int light = access->GetSkyLight(xx, yy, zz);

			bool blocksLightToSides = access->GetBlock(xx, yy, zz).GetDef()->BlocksLightToTheSides();

			// Probably is slab, they only block light going up or down
			if (yy != y && !blocksLightToSides)
			{
				light = 0;
			}

			light -= falloff;

			if (light > computedLevel)
			{
				computedLevel = light ;
			}
		}
	}
	return computedLevel;
}

int World::ComputeBlockLevel(int x, int y, int z, int falloff, CachedChunkAccess* access)
{
	static constexpr int faceOffsets[] =
	{
		0, -1, 0,
		0, 1, 0,
		-1, 0, 0,
		1, 0, 0,
		0, 0, -1,
		0, 0, 1
	};

	auto block = access->GetBlock(x, y, z).GetDef();
	if (block->GetOpacity() >= 15)
		return 0;

	int computedLevel = block->GetLightEmitted();

	if (falloff == 0) {
		falloff = 1;
	}

	for (int i = 0; i < 6; i++) {
		int xx = x + faceOffsets[i * 3 + 0];
		int yy = y + faceOffsets[i * 3 + 1];
		int zz = z + faceOffsets[i * 3 + 2];

		int light = access->GetBlockLight(xx, yy, zz);

		bool blocksLightToSides = access->GetBlock(xx, yy, zz).GetDef()->BlocksLightToTheSides();
		// Probably is slab, they only block light going up or down
		if (yy != y && !blocksLightToSides)
		{
			light = 0;
		}

		light -= falloff;

		if (light > computedLevel)
		{
			computedLevel = light;
		}
	}

	return computedLevel;
}

ChunkCoord World::GetChunkCoord(int x, int z)
{
	int64_t l = x;
	int64_t l1 = z;
	return l & 0xffffffffL | (l1 & 0xffffffffL) << 32;
}

int World::GetDimensionID()
{
	return 0;
}

bool World::HasSky()
{
	return true;
}

std::unordered_map<int64_t, std::shared_ptr<Chunk>>& World::GetChunkMap()
{
	return m_ChunkMap;
}

void World::PushTask(std::function<void()> task, bool)
{
	std::scoped_lock lock(m_TaskMutex);
	m_JobQueue.push(task);
	m_WakeCondition.notify_all();
}

void World::InitWorld()
{
	for (int i = 0; i < 16; i++)
	{
		float a = 0.05f;
		float l = 1.0f - (float)i / 15.0f;
		LightLUT[i] = (1.0f - l) / (l * 3.0f + 1.0f) * (1.0f - a) + a;
	}
}
