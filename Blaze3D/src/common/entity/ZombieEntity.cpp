#include "ZombieEntity.h"

#include "common/item/Item.h"

DECLARE_ENTITY_CONSTRUCTOR(ZombieEntity, [](World* world, TagCompound& tag) {
	return std::make_shared<ZombieEntity>(world);
});

ZombieEntity::ZombieEntity(World* world) : MobEntity(world)
{
	
}

ZombieEntity::ZombieEntity(World* world, glm::vec3 position) : MobEntity(world, position)
{
}

void ZombieEntity::OnInteractWithPlayer(BaseEntity* other)
{
	if (IsDead)
		return;
	if (other->GetUUID() == m_TargetEntity)
		Attack(other, 3);
}

std::string_view ZombieEntity::GetCreatureModel()
{
	return "zombie";
}

std::string_view ZombieEntity::GetCreatureTaunt()
{
	return "mob.zombie";
}

std::string_view ZombieEntity::GetCreatureHurtSound()
{
	return "mob.zombiehurt";
}

std::string_view ZombieEntity::GetCreatureDieSound()
{
	return "mob.zombiedeath";
}

int ZombieEntity::GetCreatureItemDrop()
{
	if (m_TargetEntity == 0)
	{
		return 0;
	}
	return Item::Feather->GetId();
}