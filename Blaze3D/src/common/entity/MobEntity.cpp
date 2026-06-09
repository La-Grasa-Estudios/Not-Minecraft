#include "MobEntity.h"

#include "common/Random.h"

#include "common/World.h"

#include "common/item/Item.h"

MobEntity::MobEntity(World* world) : CreatureEntity(world)
{
	EntityWorld = world;
}

MobEntity::MobEntity(World* world, glm::vec3 position) : MobEntity(world)
{
	SetPosition(position);
}

void MobEntity::Update()
{
	Random rand;
	CreatureEntity::Update();

	SearchEntityToAttack();

	BaseEntity* targetEntity = EntityWorld->GetEntityByUUID(m_TargetEntity);

	if (IsDead)
		return;

	if (targetEntity)
	{
		if (!targetEntity->IsDead)
		{
			if (glm::distance(targetEntity->Position, Position) > 16.0f)
			{
				m_TargetEntity = 0;
			}
			auto target = glm::ivec3(glm::floor(targetEntity->Position - glm::vec3(0.0f, targetEntity->HeightOffset, 0.0f)));
			if (m_CurrentPath.Completed || m_CurrentPath.Target != target || rand.NextInt(300) == 0)
			{
				EntityWorld->RequestPathForEntity(this, glm::ivec3(glm::floor(Position - glm::vec3(0.0f, HeightOffset, 0.0f))), target);
			}
		}
		else
		{
			m_TargetEntity = 0;
		}
	}
	else
	{
		m_TargetEntity = 0;
	}

	auto floorPos = glm::ivec3(glm::floor(Position - glm::vec3(0.0f, HeightOffset / 2.0f, 0.0f)));
	int skyLight = EntityWorld->GetSkyLightDecreased(floorPos.x, floorPos.y, floorPos.z);

	if (skyLight > 14 && m_BurnsInDaylight)
	{
		RemainingFireTicks = 300;
	}
}

void MobEntity::Attack(BaseEntity* other, int amt)
{
	if (other->ImmunityTicks > 0 || !other->CanBeHit)
		return;

	other->Velocity.y = 0.3f;
	PushEntity(other, 15.0f);
	other->TakeDamage(this, amt, DamageSource::MOB);
}

bool MobEntity::CanSpawnOn(const glm::vec3& pos)
{
	if (EntityWorld->IsOutsideWorld(floor(pos.x), floor(pos.z)) || EntityWorld->GetDimensionID() != 0)
		return false;
	auto block = EntityWorld->GetBlock((int)floor(pos.x), (int)floor(pos.y), (int)floor(pos.z)).GetDef();
	return !block->IsSolid() && block->GetId() != BLOCK_WATER && EntityWorld->GetBlock((int)floor(pos.x), (int)floor(pos.y) - 1, (int)floor(pos.z)).GetDef()->IsSolid() &&
		EntityWorld->GetMixedLight((int)floor(pos.x), (int)floor(pos.y), (int)floor(pos.z)) <= 4;
}

bool MobEntity::TakeDamage(BaseEntity* other, int amount, DamageSource source, bool skipImmunity)
{
	if (other)
	{
		m_TargetEntity = other->GetUUID();
	}
	return CreatureEntity::TakeDamage(other, amount, source, skipImmunity);
}

void MobEntity::SearchEntityToAttack()
{
	if (BaseEntity* player = EntityWorld->GetClosestPlayer(Position.x, Position.z, 16.0f); player)
	{
		if (m_TargetEntity == 0 && !player->IsDead)
		{
			if (CanSeeEntity(player))
				m_TargetEntity = player->GetUUID();
		}
	}
}
