#include "PigEntity.h"

#include "common/nbt/NBT.h"
#include "common/World.h"
#include "common/item/Item.h"

DECLARE_ENTITY_CONSTRUCTOR(PigEntity, [](World* world, TagCompound& tag) {
	return std::make_shared<PigEntity>(world);
});

PigEntity::PigEntity(World* world) : AnimalEntityBase(world)
{
	BoundingBoxWidth = 1.0f;
	BoundingBoxHeight = 0.9f;
	HeightOffset = 0.9f;
	Health = 10;
}

PigEntity::PigEntity(World* world, glm::vec3 position) : PigEntity(world)
{
	SetPosition(position);
}

void PigEntity::Update()
{
	CreatureEntity::Update();
}

std::string_view PigEntity::GetCreatureModel()
{
	return "pig";
}

std::string_view PigEntity::GetCreatureTaunt()
{
	return "mob.pig";
}

std::string_view PigEntity::GetCreatureHurtSound()
{
	return "mob.pig";
}

std::string_view PigEntity::GetCreatureDieSound()
{
	return "mob.pigdeath";
}

int PigEntity::GetCreatureItemDrop()
{
	return Item::PorkRaw->GetId();
}
