#include "CreeperEntity.h"

#include "common/World.h"
#include "common/item/Item.h"

DECLARE_ENTITY_CONSTRUCTOR(CreeperEntity, [](World* world, TagCompound& tag) {
	return std::make_shared<CreeperEntity>(world);
	});


CreeperEntity::CreeperEntity(World* world) : MobEntity(world)
{
	m_BurnsInDaylight = false;
}

CreeperEntity::CreeperEntity(World* world, glm::vec3 position) : MobEntity(world, position)
{
}

void CreeperEntity::OnInteractWithPlayer(BaseEntity* other)
{

}

void CreeperEntity::Update()
{
	MobEntity::Update();

	m_FollowPath = true;

	if (m_TargetEntity)
	{
		auto target = EntityWorld->GetEntityByUUID(m_TargetEntity);

		bool canSee = CanSeeEntity(target);

		if (canSee && glm::distance(target->Position, Position) < 3.0f)
		{
			m_FollowPath = false;

			if (m_Fuse == 0)
			{
				EntityWorld->PlaySound("mob.creeper.fuse", Position, 0.4f);
			}

			m_Fuse++;
		}
		else if (canSee)
		{
			if (glm::distance(target->Position, Position) > 4.0f)
			{
				m_Fuse = 0;
			}
		}
	}
	else
	{
		m_Fuse = 0;
	}

	if (m_Fuse > 30)
	{
		ForceRemove = true;
		EntityWorld->DoExplosion(this, Position.x, Position.y, Position.z, 3.0f);
	}

	if (m_Fuse != 0 && m_Fuse % 10 < 5)
	{
		m_CreeperState = 1;
	}
	else
	{
		m_CreeperState = 0;
	}
}

std::string_view CreeperEntity::GetCreatureModel()
{
	return "creeper";
}

std::string_view CreeperEntity::GetCreatureTaunt()
{
	return "";
}

std::string_view CreeperEntity::GetCreatureHurtSound()
{
	return "mob.creeper.hurt";
}

std::string_view CreeperEntity::GetCreatureDieSound()
{
	return "mob.creeper.death";
}

int CreeperEntity::GetCreatureItemDrop()
{
	return Item::Gunpowder->GetId();
}
