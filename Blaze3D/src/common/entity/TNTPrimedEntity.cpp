#include "TNTPrimedEntity.h"

#include "common/Random.h"

#include "common/World.h"
#include "common/nbt/NBT.h"

DECLARE_ENTITY_CONSTRUCTOR(TNTPrimedEntity, [](World* world, TagCompound& tag) {
	return std::make_shared<TNTPrimedEntity>(world);
});


TNTPrimedEntity::TNTPrimedEntity(World* world)
{
	EntityWorld = world;
	Dimension = world->GetDimensionID();
	BoundingBoxWidth = 0.98f;
	BoundingBoxHeight = 0.98f;
	HeightOffset = 0.0f;
	CanBeHit = false;
}

TNTPrimedEntity::TNTPrimedEntity(World* world, int x, int y, int z) : TNTPrimedEntity(world)
{
	float angle = Random::GlobalRand.NextFloat() * glm::two_pi<float>();
	Velocity.x = -glm::sin(angle * glm::pi<float>() / 180.0f) * 0.02f;
	Velocity.y = 0.2f;
	Velocity.z = -glm::cos(angle * glm::pi<float>() / 180.0f) * 0.02f;
	Timer = 80;
	SetPosition(glm::vec3(x, y - 0.5f, z) + glm::vec3(0.5f));
}

void TNTPrimedEntity::Update()
{

	Velocity.y -= 0.04f;
	Move();
	Velocity.x *= 0.98f;
	Velocity.y *= 0.98f;
	Velocity.z *= 0.98f;

	EntityWorld->SpawnParticle("smoke", Position.x, Position.y + 1.0f, Position.z);

	if (IsGrounded)
	{
		Velocity.x *= 0.7f;
		Velocity.z *= 0.7f;
		Velocity.y *= -0.5f;
	}
	
	Timer--;
	if (Timer < 0)
	{
		EntityWorld->DoExplosion(this, Position.x, Position.y, Position.z, 4.0f);
		ForceRemove = true;
	}
}

void TNTPrimedEntity::WriteToNbt(TagCompound& tag)
{
	BaseEntity::WriteToNbt(tag);
}

void TNTPrimedEntity::ReadFromNbt(TagCompound& tag)
{
	BaseEntity::ReadFromNbt(tag);
}
