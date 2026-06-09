#include "BlockFire.h"

#include "common/World.h"
#include "common/PortalStructure.h"
	
#include "common/Random.h"

int GetChanceOfFire(World* world, int x, int y, int z, int chance)
{
	int newChance = world->GetBlock(x, y, z).GetDef()->GetChanceOfFire();
	return newChance > chance ? newChance : chance;
}

bool AreNeighboursFlammable(World* world, int x, int y, int z)
{
	return  world->GetBlock(x + 1, y, z).GetDef()->IsFlammable() ||
			world->GetBlock(x - 1, y, z).GetDef()->IsFlammable() ||
			world->GetBlock(x, y - 1, z).GetDef()->IsFlammable() ||
			world->GetBlock(x, y + 1, z).GetDef()->IsFlammable() ||
			world->GetBlock(x, y, z - 1).GetDef()->IsFlammable() ||
			world->GetBlock(x, y, z + 1).GetDef()->IsFlammable();
}

void TryCatchBlockOnFire(World* world, int x, int y, int z, int chance)
{
	int ability = world->GetBlock(x, y, z).GetDef()->GetAbilityToCatchFire();
	if (Random::GlobalRand.NextInt(chance) < ability * 2)
	{
		if (Random::GlobalRand.NextInt(2) == 0)
		{
			world->SetBlock(x, y, z, BLOCK_FIRE);
		}
		else
		{
			world->SetBlock(x, y, z, BLOCK_AIR);
		}
	}
}

int GetChanceOfFireOnNeighbours(World* world, int x, int y, int z)
{
	if (world->GetBlock(x, y, z).Id != 0)
	{
		return 0;
	}
	else
	{
		int chance = GetChanceOfFire(world, x + 1, y, z, 0);
		chance = GetChanceOfFire(world, x - 1, y, z, chance);
		chance = GetChanceOfFire(world, x, y + 1, z, chance);
		chance = GetChanceOfFire(world, x, y - 1, z, chance);
		chance = GetChanceOfFire(world, x, y, z + 1, chance);
		chance = GetChanceOfFire(world, x, y, z - 1, chance);
		return chance;
	}
}

BlockFire::BlockFire(uint8_t id) : Block(id, 31)
{
	SetIsSolid(false);
	SetIsSolidToRaycast(false);
	SetLightEmitted(15);
	SetRenderAsIcon(true);
	SetIconIndex(31);
	SetRenderNeighbours(true);
	SetOpacity(0);
	SetIsReplaceableByPlayer(true);
}

void BlockFire::Tick(World* world, int x, int y, int z)
{
	if (PortalStructure::IsPortalValid(world, x, y, z))
	{
		PortalStructure::FillPortal(world, x, y, z);
	}

	if (!CanExistAt(world, x, y, z, -1, -1))
	{
		world->SetBlock(x, y, z, BLOCK_AIR);
	}
}

void BlockFire::RandomTick(World* world, int x, int y, int z)
{
	bool eternalFire = world->GetBlock(x, y - 1, z).Id == BLOCK_NETHERRACK;
	auto current = world->GetBlock(x, y, z);
	int fireAge = current.Metadata;
	if (fireAge < 15)
	{
		current.Metadata += 1;
		world->SetBlock(x, y, z, current);
		fireAge++;
	}

	if (!eternalFire && !AreNeighboursFlammable(world, x, y, z))
	{
		if (world->GetBlock(x, y - 1, z).GetDef()->MaxY < 0.99f || fireAge > 3)
		{
			world->SetBlock(x, y, z, BLOCK_AIR);
		}
	}
	else if (!eternalFire && !world->GetBlock(x, y - 1, z).GetDef()->IsFlammable() && fireAge == 15 && Random::GlobalRand.NextInt(4) == 0)
	{
		world->SetBlock(x, y, z, BLOCK_AIR);
	}
	else
	{
		TryCatchBlockOnFire(world, x + 1, y, z, 300);
		TryCatchBlockOnFire(world, x - 1, y, z, 300);
		TryCatchBlockOnFire(world, x, y + 1, z, 250);
		TryCatchBlockOnFire(world, x, y - 1, z, 250);
		TryCatchBlockOnFire(world, x, y, z + 1, 300);
		TryCatchBlockOnFire(world, x, y, z - 1, 300);

		for (int xx = x - 1; xx <= x + 1; xx++)
		{
			for (int yy = y - 1; yy <= y + 4; yy++)
			{
				for (int zz = z - 1; zz <= z + 1; zz++)
				{
					if (xx != x || yy != y || zz != z)
					{
						int chance = 100;
						if (yy > y + 1)
						{
							chance += (yy - (y + 1)) * 100;
						}

						int neighbourChance = GetChanceOfFireOnNeighbours(world, xx, yy, zz);
						if (neighbourChance > 0 && Random::GlobalRand.NextInt(chance) <= neighbourChance)
						{
							world->SetBlock(xx, yy, zz, BLOCK_FIRE);
						}
					}
				}
			}
		}
	}
	Tick(world, x, y, z);
}

bool BlockFire::CanExistAt(World* world, int x, int y, int z, int faceId, int facing)
{
	if (world->GetBlock(x - 1, y, z).GetDef()->IsFlammable())
		return true;
	if (world->GetBlock(x + 1, y, z).GetDef()->IsFlammable())
		return true;
	if (world->GetBlock(x, y, z - 1).GetDef()->IsFlammable())
		return true;
	if (world->GetBlock(x, y, z + 1).GetDef()->IsFlammable())
		return true;
	if (world->GetBlock(x, y - 1, z).GetDef()->IsFlammable())
		return true;
	if (world->GetBlock(x, y + 1, z).GetDef()->IsFlammable())
		return true;
	return world->GetBlock(x, y - 1, z).GetDef()->MaxY >= 0.99f && world->GetBlock(x, y - 1, z).GetDef()->IsSolid();
}

void BlockFire::DisplayTick(World* world, int x, int y, int z)
{
	if (Random::GlobalRand.NextInt(24) == 0)
	{
		world->PlaySound("misc.fire", glm::vec3(x, y, z) + glm::vec3(0.5f), 0.2f);
	}

	auto below = world->GetBlock(x, y - 1, z).GetDef();

	if (((below->MaxY <= 0.99f) || !below->IsSolid()) && !below->IsFlammable())
	{
		if (world->GetBlock(x - 1, y, z).GetDef()->IsFlammable()) {
			for (int i = 0; i < 2; ++i) {
				float px = (float)x + Random::GlobalRand.NextFloat() * 0.1F;
				float py = (float)y + Random::GlobalRand.NextFloat();
				float pz = (float)z + Random::GlobalRand.NextFloat();
				world->SpawnParticle("smoke", px, py, pz);
			}
		}

		if (world->GetBlock(x + 1, y, z).GetDef()->IsFlammable()) {
			for (int i = 0; i < 2; ++i) {
				float px = (float)(x + 1) - Random::GlobalRand.NextFloat() * 0.1F;
				float py = (float)y + Random::GlobalRand.NextFloat();
				float pz = (float)z + Random::GlobalRand.NextFloat();
				world->SpawnParticle("smoke", px, py, pz);
			}
		}

		if (world->GetBlock(x, y, z - 1).GetDef()->IsFlammable()) {
			for (int i = 0; i < 2; ++i) {
				float px = (float)x + Random::GlobalRand.NextFloat();
				float py = (float)y + Random::GlobalRand.NextFloat();
				float pz = (float)z + Random::GlobalRand.NextFloat() * 0.1F;
				world->SpawnParticle("smoke", px, py, pz);
			}
		}

		if (world->GetBlock(x, y, z + 1).GetDef()->IsFlammable()) {
			for (int i = 0; i < 2; ++i) {
				float px = (float)x + Random::GlobalRand.NextFloat();
				float py = (float)y + Random::GlobalRand.NextFloat();
				float pz = (float)(z + 1) - Random::GlobalRand.NextFloat() * 0.1F;
				world->SpawnParticle("smoke", px, py, pz);
			}
		}

		if (world->GetBlock(x, y + 1, z).GetDef()->IsFlammable()) {
			for (int i = 0; i < 2; ++i) {
				float px = (float)x + Random::GlobalRand.NextFloat();
				float py = (float)(y + 1) - Random::GlobalRand.NextFloat() * 0.1F;
				float pz = (float)z + Random::GlobalRand.NextFloat();
				world->SpawnParticle("smoke", px, py, pz);
			}
		}
	}
	else
	{
		for (int i = 0; i < 3; i++) 
		{
			float px = x + Random::GlobalRand.NextFloat();
			float py = y + Random::GlobalRand.NextFloat() * 0.5f + 0.5f;
			float pz = z + Random::GlobalRand.NextFloat();
			world->SpawnParticle("smoke", px, py, pz);
		}
	}
}

BlockRenderType BlockFire::GetRenderType()
{
	return BLOCK_RENDER_TYPE_FIRE;
}
