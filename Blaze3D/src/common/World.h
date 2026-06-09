#pragma once

#include "Chunk.h"
#include "SaveManager.h"
#include "entity/AABB.h"

#include "WorldCommon.h"
#include "IWorldAccess.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

#define TICK_LIST_SIZE 1024
#define TICK_TYPE_NORMAL 0
#define TICK_TYPE_LIQUID 1

class BaseEntity;
class WorldRenderer;
class CachedChunkAccess;
class TagCompound;
class TileEntity;
class ChunkProvider;
struct Particle;

typedef uint64_t ChunkCoord;

struct ScheduledTick
{
	int x, y, z;
	uint16_t delay, type;
};

struct LightQueueEntry
{
	int x, y, z;
	uint16_t lightLevel, startingLightLevel;
};

class World
{
public:
	friend WorldRenderer;
	friend SaveManager;

	float LightLUT[16];

	World(const std::string& path);
	~World();
	void Generate();
	void PreGenerate(int radius);
	void SearchValidSpawn();
	void RequestChunksSurroundingArea(int x, int z, int loadDistance, bool locking = false, std::function<void(float)> cb = NULL);
	void ProcessWorldGenQueue();

	Chunk* GetChunk(ChunkCoord coord);
	Chunk* GetChunk(int x, int z);
	bool AreChunksLoaded(int x, int z, int rd, TerrainGenerationStage stage);

	void SetBlock(int x, int y, int z, BlockStorage block);
	void SetBlockNoNotify(int x, int y, int z, BlockStorage block);
	void SetBlockNoNotifyNoDirty(int x, int y, int z, BlockStorage block);

	void SetDirtyFlag(int x, int y, int z);
	void NotifyBlock(int x, int y, int z);
	void NotifyNeighboursWithDelay(int x, int y, int z, uint16_t delay, uint16_t type);
	void ScheduleUpdate(int x, int y, int z, uint16_t delay, uint16_t type);

	void UpdateSkyLightColumn(int x, int z, CachedChunkAccess* access);
	void UpdateBlockLight(int x, int y, int z, CachedChunkAccess* access);

	bool IsOutsideWorld(int x, int z);

	BlockStorage GetBlock(int x, int y, int z);
	int GetHeight(int x, int z);

	void SetSkyLight(int x, int y, int z, int level);
	int GetSkyLight(int x, int y, int z);
	int GetSkyLightDecreased(int x, int y, int z);

	void SetBlockLight(int x, int y, int z, int level);
	int GetBlockLight(int x, int y, int z);

	int GetMixedLight(int x, int y, int z);
	int GetMixedLightNoDecrease(int x, int y, int z);

	void SetTileEntity(int x, int y, int z, std::shared_ptr<TileEntity> entity);
	TileEntity* GetTileEntity(int x, int y, int z);

	void GetBoxes(AABB box, AABB* results, int& count);
	BlockLiquidType IsBoundingBoxInLiquid(AABB box);
	bool IsBoundingBoxFree(AABB box);
	// Gets all colliding entities with the provided boundingBox, NOT THREAD SAFE
	BaseEntity** GetCollidingEntitiesWith(AABB bb, int* count);
	// Gets all entities colliding with the provided ray checks for block collision, NOT THREAD SAFE
	BaseEntity* GetEntityWithRayCast(BaseEntity* src, const glm::vec3& org, const glm::vec3& dir, float maxDistance);
	BaseEntity* GetEntityWithRayCast(BaseEntity* src, BaseEntity* dst, const glm::vec3& org, const glm::vec3& dir, float maxDistance);
	BaseEntity* GetClosestPlayer(float x, float z, float distance);
	BaseEntity* GetEntityByUUID(uint64_t uuid);
	bool RayCast(const glm::vec3& org, const glm::vec3& dir, float maxDistance, RayCastHit& out);
	bool RayCastLiquids(const glm::vec3& org, const glm::vec3& dir, float maxDistance, RayCastHit& out);

	void RequestPathForEntity(BaseEntity* entity, const glm::ivec3& start, const glm::ivec3& goal);

	float GetBoundingBoxVisibilityFromPoint(const glm::vec3& point, AABB bb);

	void DoExplosion(BaseEntity* owner, float x, float y, float z, float power);

	template<typename T>
	int CountEntitiesOfType()
	{
		int count = 0;
		for (int i = 0; i < m_Entities.size(); i++)
		{
			if (dynamic_cast<T*>(m_Entities[i].get()) != nullptr)
			{
				count++;
			}
		}
		return count;
	}
	void AddEntity(std::shared_ptr<BaseEntity> entity);
	bool CanAddEntity();

	AABB GetBlockBoundingBox(int x, int y, int z);

	void Update();
	void NetworkUpdate();

	std::vector<std::shared_ptr<BaseEntity>>& GetAllEntities();
	size_t GetEntityCount();
	void WriteToNbt(TagCompound& root);
	void LoadFromNbt(TagCompound& root);

	void UpdateInternalSkylightDecrease(float partialTicks);
	float GetCelestialAngle(float partialTicks);
	inline float GetDayProgress();
	inline int GetWorldTime();
	void SetWorldTime(int time);

	std::vector<std::shared_ptr<BaseEntity>> GetEntitiesInChunk(Chunk* chunk);
	std::vector<BaseEntity*> GetPlayers();

	void Close();
	bool Load(std::function<void(float)> cb);
	void Save(std::function<void(float)> cb);

	SaveManager& GetSaveManager();
	virtual ChunkProvider& GetChunkProvider();

	Particle* SpawnParticle(std::string_view name, float x, float y, float z, TagCompound* extraData = NULL);
	void PlaySound(std::string_view name, float x, float y, float z, float volume);
	void PlaySound(std::string_view name, const glm::vec3& position, float volume);
	void PlaySoundEffect(std::string_view name, float volume);

	IWorldAccess* WorldAccess = NULL;

	uint64_t Seed;

	int SpawnX, SpawnY, SpawnZ;

	static ChunkCoord GetChunkCoord(int x, int z);
	virtual int GetDimensionID();
	virtual bool HasSky();

	bool IsServerWorld = false;

	std::unordered_map<int64_t, std::shared_ptr<Chunk>>& GetChunkMap();

protected:

	void PushTask(std::function<void()> task, bool isServerWorld);

	virtual void InitWorld();

	ChunkProvider* m_ChunkProvider = NULL;

	int m_InternalSkylightDecrease = 0;
	int m_Ticks = 0;

	SaveManager m_SaveManager;

	void UpdateSkyLight(int x, int y, int z, CachedChunkAccess* access);
	int ComputeSkyLevel(int x, int y, int z, int falloff, CachedChunkAccess* access);
	int ComputeBlockLevel(int x, int y, int z, int falloff, CachedChunkAccess* access);

	std::vector<std::shared_ptr<BaseEntity>> m_Entities;
	std::vector<std::shared_ptr<BaseEntity>> m_AliveEntities;
	std::vector<BaseEntity*> m_CollidingEntities;
	std::vector<AABB> m_CollidingBoxes;

	std::queue<std::shared_ptr<Chunk>> m_ChunkGenQueue;
	std::vector<ScheduledTick> m_TickList;
	std::queue<ScheduledTick> m_DelayedTickList;
	std::queue<LightQueueEntry> m_SkyLightQueue;
	std::queue<std::function<void()>> m_JobQueue;

	std::condition_variable m_WakeCondition;
	std::mutex wakeMutex;
	std::mutex m_TaskMutex;

	std::atomic_bool m_IsWorldThreadRunning = false;
	std::atomic_bool m_IsWorldActive = false;

	std::unordered_map<int64_t, std::shared_ptr<Chunk>> m_ChunkMap;

};