#include "CowEntity.h"

#include "common/nbt/NBT.h"
#include "common/World.h"
#include "common/item/Item.h"

DECLARE_ENTITY_CONSTRUCTOR(CowEntity, [](World* world, TagCompound& tag) {
	return std::make_shared<CowEntity>(world);
});

CowEntity::CowEntity(World* world) : AnimalEntityBase(world)
{
	BoundingBoxWidth = 1.0f;
	BoundingBoxHeight = 1.3f;
	HeightOffset = 1.3f;
	Health = 10;
}

CowEntity::CowEntity(World* world, glm::vec3 position) : CowEntity(world)
{
	SetPosition(position);
}

std::string_view CowEntity::GetCreatureModel()
{
	return "cow";
}

std::string_view CowEntity::GetCreatureTaunt()
{
	return "mob.cow";
}

std::string_view CowEntity::GetCreatureHurtSound()
{
	return "mob.cowhurt";
}

std::string_view CowEntity::GetCreatureDieSound()
{
	return "mob.cowhurt";
}

int CowEntity::GetCreatureItemDrop()
{
	return Item::Leather->GetId();
}
