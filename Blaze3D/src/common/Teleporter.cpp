#include "Teleporter.h"

#include "common/entity/BaseEntity.h"
#include "common/World.h"
#include "common/WorldHell.h"

#include "client/Client.h"

void Teleporter::TeleportEntity(BaseEntity* entity)
{
	if (entity->Dimension != -1)
	{
		entity->Dimension = -1;
		entity->Position /= glm::vec3(8.0f, 1.0f, 8.0f);
		Client::GetInstance().SwitchLevel(new WorldHell(entity->EntityWorld->GetSaveManager().GetSavePath()));
	}
	else
	{
		entity->Dimension = 0;
		entity->Position *= glm::vec3(8.0f, 1.0f, 8.0f);
		Client::GetInstance().SwitchLevel(new World(entity->EntityWorld->GetSaveManager().GetSavePath()));
	}

	int x = (int)floor(entity->Position.x);
	int z = (int)floor(entity->Position.z);
	entity->EntityWorld->RequestChunksSurroundingArea(x, z, 6, true, [](float p)
	{
		Client::GetInstance().RenderLoadingScreenWithBar("Teleporting", p);
	});

	auto world = entity->EntityWorld;

	bool foundPortal = false;
	int r = 128;
	int minX = x - r, minZ = z - r;
	int maxX = x + r, maxZ = z + r;
	for (int xx = minX; xx <= maxX && !foundPortal; xx++)
	{
		for (int zz = minZ; zz <= maxZ && !foundPortal; zz++)
		{
			for (int y = 128; y > 0 && !foundPortal; y--)
			{
				auto block = world->GetBlock(xx, y, zz);
				if (block.Id == BLOCK_PORTAL)
				{
					for (int i = 0; i < 10; i++)
					{
						if (world->GetBlock(xx, y - i, zz).GetDef()->GetId() != BLOCK_PORTAL)
						{
							entity->SetPosition(glm::vec3(xx + 0.5f, y - i + 1 + entity->HeightOffset, zz + 0.5f));
							foundPortal = true;
							break;
						}
					}
				}
			}
		}
	}

	if (!foundPortal)
	{
		auto tryGeneratePortal = [&](World* world, int x, int y, int z, bool force)
			{
				const int PORTAL_WIDTH = 4;
				const int PORTAL_HEIGHT = 5;

				if (!force)
				{
					for (int px = 0; px < PORTAL_WIDTH; px++)
					{
						for (int py = 0; py < PORTAL_HEIGHT; py++)
						{
							bool isInsideFrame = px >= 1 && px <= PORTAL_WIDTH - 2 && py >= 1 && py <= PORTAL_HEIGHT - 2;
							auto block = world->GetBlock(x + px, y + py, z).GetDef();
							if (block->IsSolid() || block->GetLiquidType() != 0 || !block->IsReplaceableByPlayer())
							{
								return false;
							}
							if (py == 0)
							{
								block = world->GetBlock(x + px, y + py - 1, z).GetDef();
								if (block->GetLiquidType() != 0 || !block->IsSolid())
								{
									return false;
								}
							}
						}
					}
				}

				for (int px = 0; px < PORTAL_WIDTH; px++)
				{
					for (int py = 0; py < PORTAL_HEIGHT; py++)
					{
						bool isInsideFrame = px >= 1 && px <= PORTAL_WIDTH - 2 && py >= 1 && py <= PORTAL_HEIGHT - 2;
						if (isInsideFrame)
						{
							world->SetBlock(x + px, y + py, z, BLOCK_PORTAL);
						}
						else
						{
							world->SetBlock(x + px, y + py, z, BLOCK_OBSIDIAN);
						}
					}
				}

				return true;
			};

		r = 12;

		minX = x - r;
		minZ = z - r;
		maxX = x + r;
		maxZ = z + r;

		for (int xx = minX; xx <= maxX && !foundPortal; xx++)
		{
			for (int zz = minZ; zz <= maxZ && !foundPortal; zz++)
			{
				for (int y = 120; y > 0 && !foundPortal; y--)
				{
					auto block = world->GetBlock(xx, y, zz).GetDef();
					if (block->IsSolid() && block->GetLiquidType() == 0)
					{
						if (tryGeneratePortal(world, xx, y + 1, zz, false))
						{
							printf("Generated portal at %i %i %i\n", xx, y, zz);
							entity->SetPosition(glm::vec3(xx + 1.5f, y + 2 + entity->HeightOffset, zz + 0.5f));
							return;
						}
					}
				}
			}
		}

		// We could'nt find any portal or generate one, force one at destination

		int y = (int)floor(entity->Position.y);

		const int AIR_REGION_WIDTH = 2;
		const int AIR_REGION_HEIGHT = 4; // 3 air blocks + 1 for obsidian platform

		for (int xx = -1; xx <= 1; xx++)
		{
			for (int zz = AIR_REGION_WIDTH; zz <= AIR_REGION_WIDTH; zz++)
			{
				for (int yy = 0; yy < AIR_REGION_HEIGHT; yy++)
				{
					if (yy == 0)
					{
						world->SetBlock(xx + x, yy + y, zz + z, BLOCK_OBSIDIAN);
					} else
					{
						world->SetBlock(xx + x, yy + y, zz + z, BLOCK_AIR);
					}
				}
			}
		}

		tryGeneratePortal(world, x, y, z, true);
		entity->SetPosition(glm::vec3(x + 1.5f, y + 2 + entity->HeightOffset, z + 0.5f));
		return;
	}
	else
	{
		return;
	}

	// This should'nt happen, fallback to spawn
	entity->SetPosition(glm::vec3(world->SpawnX + 0.5f, world->SpawnY + 0.5f, world->SpawnZ));
}
