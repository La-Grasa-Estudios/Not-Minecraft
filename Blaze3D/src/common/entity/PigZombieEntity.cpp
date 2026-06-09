#include "PigZombieEntity.h"

#include "common/Random.h"

#include "common/World.h"
#include "common/item/Item.h"

DECLARE_ENTITY_CONSTRUCTOR(PigZombieEntity, [](World* world, TagCompound& tag) {
	return std::make_shared<PigZombieEntity>(world);
	});

PigZombieEntity::PigZombieEntity(World* world) : MobEntity(world)
{

}

PigZombieEntity::PigZombieEntity(World* world, glm::vec3 position) : MobEntity(world, position)
{
}

void PigZombieEntity::OnInteractWithPlayer(BaseEntity* other)
{
	if (IsDead)
		return;
	if (other->GetUUID() == m_TargetEntity)
		Attack(other, 8);
}

bool PigZombieEntity::TakeDamage(BaseEntity* other, int amount, DamageSource source, bool skipImmunity)
{
	if (other)
	{
		int count;
		auto entities = EntityWorld->GetCollidingEntitiesWith(BoundingBox.grow(32.0f, 32.0f, 32.0f), &count);

		for (int i = 0; i < count; i++)
		{
			auto entity = entities[i];
			if (auto pigzombie = dynamic_cast<PigZombieEntity*>(entity); pigzombie)
			{
				pigzombie->BecomeAngry(other);
			}
		}

		this->BecomeAngry(other);
	}

	return MobEntity::TakeDamage(other, amount, source, skipImmunity);
}

bool PigZombieEntity::CanSpawnOn(const glm::vec3& pos)
{
	if (EntityWorld->IsOutsideWorld(floor(pos.x), floor(pos.z)) || EntityWorld->GetDimensionID() != -1)
		return false;
	auto block = EntityWorld->GetBlock((int)floor(pos.x), (int)floor(pos.y), (int)floor(pos.z)).GetDef();
	return !block->IsSolid() &&
		block->GetLiquidType() == 0 &&
		EntityWorld->GetBlock((int)floor(pos.x), (int)floor(pos.y) - 1, (int)floor(pos.z)).Id == BLOCK_NETHERRACK;
}

void PigZombieEntity::Update()
{
	MobEntity::Update();
	if (m_AngerLevel > 0)
	{
		m_AngerLevel--;
	}
	if (m_AngrySoundDelay > 0 && --m_AngrySoundDelay == 0)
	{
		EntityWorld->PlaySound("mob.zombiepig.angry", Position, 0.5f);
	}
}

void PigZombieEntity::BecomeAngry(BaseEntity* target)
{
	m_AngerLevel = 400 + Random::GlobalRand.NextInt(400);
	m_AngrySoundDelay = Random::GlobalRand.NextInt(40);
	m_TargetEntity = target->GetUUID();
}

void PigZombieEntity::SearchEntityToAttack()
{
	if (m_AngerLevel > 0)
	{
		MobEntity::SearchEntityToAttack();
	}
}

std::string_view PigZombieEntity::GetCreatureModel()
{
	return "pigzombie";
}

std::string_view PigZombieEntity::GetCreatureTaunt()
{
	return "mob.zombiepig";
}

std::string_view PigZombieEntity::GetCreatureHurtSound()
{
	return "mob.zombiepighurt";
}

std::string_view PigZombieEntity::GetCreatureDieSound()
{
	return "mob.zombiepigdeath";
}

int PigZombieEntity::GetCreatureItemDrop()
{
	if (m_TargetEntity == 0)
	{
		return 0;
	}
	return Item::PorkCooked->GetId();
}