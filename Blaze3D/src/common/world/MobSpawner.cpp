#include "MobSpawner.h"

#include "common/Random.h"

#include "common/World.h"
#include "common/entity/BaseEntity.h"
#include "common/entity/AnimalEntityBase.h"
#include "common/entity/MobEntity.h"

#include <unordered_set>

constexpr int MOB_COW = 1;
constexpr int LIMIT_MOB_ANIMAL = 20;
constexpr int LIMIT_MOB_ZOMBIE = 50;

static std::unordered_set<ChunkCoord> eligibleChunksForSpawn;

static std::vector<std::vector<int>> mobSpawnLimits =
{
	{ LIMIT_MOB_ANIMAL, LIMIT_MOB_ANIMAL, LIMIT_MOB_ANIMAL },
};

static std::vector<std::vector<const char*>> mobSpawnList =
{
	{ "CowEntity", "PigEntity", "ChickenEntity" },
};

static std::vector<std::vector<int>> monsterSpawnLimits =
{
	{ LIMIT_MOB_ZOMBIE, LIMIT_MOB_ZOMBIE, LIMIT_MOB_ZOMBIE },
};

static std::vector<std::vector<const char*>> monsterSpawnList =
{
	{ "ZombieEntity", "CreeperEntity", "PigZombieEntity"},
};

void PerformMobSpawningMonsters(World* world)
{
	for (int i = 0; i < monsterSpawnList.size(); i++)
	{
		auto& list = monsterSpawnList[i];
		for (int j = 0; j < list.size(); j++)
		{
			int limit = monsterSpawnLimits[i][j];
#ifdef __wii__
			limit /= 4;
#pragma message "Wii version monster cap is 4 times less than pc"
#endif
			int count = world->CountEntitiesOfType<MobEntity>();

			//printf("There are %i entities of type MobEntity\n", count);

			Random random;

			for (auto chunkCoord : eligibleChunksForSpawn)
			{
				if (count >= limit)
					break;

				if (random.NextInt(50) != 0)
					continue;

				Chunk* chunk = world->GetChunk(chunkCoord);
				if (!chunk || chunk->GenStage != TERRAIN_GEN_COMPLETE)
					continue;

				int baseX = chunk->GetStartX();
				int baseZ = chunk->GetStartZ();
				int x = baseX + random.NextInt(16);
				int y = random.NextInt(CHUNK_HEIGHT);
				int z = baseZ + random.NextInt(16);

				if (world->GetBlock(x, y, z).GetDef()->IsSolid())
				{
					continue;
				}

				int spawned = 0;

				for (int k = 0; k < 3 && spawned < 4; k++)
				{
					const int spawnRadius = 6;
					for (int l = 0; l < 4; l++)
					{
						x += random.NextInt(spawnRadius) - random.NextInt(spawnRadius);
						y += random.NextInt(1) - random.NextInt(1);
						z += random.NextInt(spawnRadius) - random.NextInt(spawnRadius);

						auto at = world->GetBlock(x, y, z).GetDef();
						auto below = world->GetBlock(x, y - 1, z).GetDef();
						auto up = world->GetBlock(x, y + 1, z).GetDef();

						if (below->IsSolid() && !at->IsSolid() && !at->GetId() != BLOCK_WATER && !at->GetId() != BLOCK_LAVA && !up->IsSolid())
						{
							float xx = x + 0.5f;
							float yy = y;
							float zz = z + 0.5f;
							if (!world->GetClosestPlayer(xx, zz, 24.0f))
							{
								float distanceToSpawn = glm::distance(glm::vec3(xx, yy, zz), glm::vec3(world->SpawnX, world->SpawnY, world->SpawnZ));
								if (distanceToSpawn >= 24.0f)
								{
									auto entity = BaseEntity::CreateEntityByType(list[j], world);
									entity->SetPosition({ xx, yy + entity->HeightOffset, zz });
									entity->Rotation.y = random.NextFloat() * 360.0f;
									if (static_cast<MobEntity*>(entity.get())->CanSpawnOn({ xx, yy, zz }))
									{
										world->AddEntity(entity);
										spawned++;
										//printf("Spawned monster %s at %.2f, %.2f, %.2f\n", list[j], xx, yy, zz);
									}

									if (spawned >= 4)
									{
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void PerformMobSpawningAnimals(World* world)
{
	for (int i = 0; i < mobSpawnList.size(); i++)
	{
		auto& list = mobSpawnList[i];
		for (int j = 0; j < list.size(); j++)
		{
			int limit = mobSpawnLimits[i][j];
			int count = world->CountEntitiesOfType<AnimalEntityBase>();

			Random random;

			for (auto chunkCoord : eligibleChunksForSpawn)
			{
				if (count >= limit)
					break;

				if (random.NextInt(50) != 0)
					continue;

				Chunk* chunk = world->GetChunk(chunkCoord);
				if (!chunk || chunk->GenStage != TERRAIN_GEN_COMPLETE)
					continue;

				int baseX = chunk->GetStartX();
				int baseZ = chunk->GetStartZ();
				int x = baseX + random.NextInt(16);
				int y = random.NextInt(CHUNK_HEIGHT);
				int z = baseZ + random.NextInt(16);

				if (world->GetBlock(x, y, z).GetDef()->IsSolid())
				{
					continue;
				}

				int spawned = 0;

				for (int k = 0; k < 3 && spawned < 4; k++)
				{
					const int spawnRadius = 6;
					for (int l = 0; l < 4; l++)
					{
						x += random.NextInt(spawnRadius) - random.NextInt(spawnRadius);
						y += random.NextInt(1) - random.NextInt(1);
						z += random.NextInt(spawnRadius) - random.NextInt(spawnRadius);

						auto at = world->GetBlock(x, y, z).GetDef();
						auto below = world->GetBlock(x, y - 1, z).GetDef();
						auto up = world->GetBlock(x, y + 1, z).GetDef();

						if (below->IsSolid() && !at->IsSolid() && !at->GetId() != BLOCK_WATER && !at->GetId() != BLOCK_LAVA && !up->IsSolid())
						{
							float xx = x + 0.5f;
							float yy = y;
							float zz = z + 0.5f;
							if (!world->GetClosestPlayer(xx, zz, 24.0f))
							{
								float distanceToSpawn = glm::distance(glm::vec3(xx, yy, zz), glm::vec3(world->SpawnX, world->SpawnY, world->SpawnZ));
								if (distanceToSpawn >= 24.0f)
								{
									auto entity = BaseEntity::CreateEntityByType(list[j], world);
									entity->SetPosition({ xx, yy + entity->HeightOffset, zz });
									entity->Rotation.y = random.NextFloat() * 360.0f;
									if (static_cast<CreatureEntity*>(entity.get())->CanSpawnOn({ xx, yy, zz }))
									{
										world->AddEntity(entity);
										spawned++;
									}

									if (spawned >= 4)
									{
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void MobSpawner::PerformMobSpawning(World* world)
{
	eligibleChunksForSpawn.clear();

	const int perPlayerRadius = 8;
	auto players = world->GetPlayers();

	for (auto player : players)
	{
		int playerChunkX = (int)floor(player->Position.x / 16.0f);
		int playerChunkZ = (int)floor(player->Position.z / 16.0f);

		for (int x = -perPlayerRadius; x <= perPlayerRadius; x++)
		{
			for (int z = -perPlayerRadius; z <= perPlayerRadius; z++)
			{
				ChunkCoord chunkCoord = World::GetChunkCoord(playerChunkX + x, playerChunkZ + z);
				eligibleChunksForSpawn.insert(chunkCoord);
			}
		}
	}

	PerformMobSpawningAnimals(world);
	PerformMobSpawningMonsters(world);
}
