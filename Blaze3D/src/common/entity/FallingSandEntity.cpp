#include "FallingSandEntity.h"

#include "common/World.h"
#include "common/nbt/NBT.h"

#include <thirdparty/glm/ext.hpp>

DECLARE_ENTITY_CONSTRUCTOR(FallingSandEntity, [](World* world, TagCompound& tag) {
	return std::make_shared<FallingSandEntity>(world);
});

FallingSandEntity::FallingSandEntity(World* world) : BaseEntity()
{
	EntityWorld = world;
	Dimension = world->GetDimensionID();
	BoundingBoxWidth = 0.98f;
	BoundingBoxHeight = 0.98f;
	HeightOffset = 0.0f;
	CanBeHit = false;
	AllowSliding = false;
}

FallingSandEntity::FallingSandEntity(World* world, int x, int y, int z) : FallingSandEntity(world)
{
	auto block = world->GetBlock(x, y, z);
	world->SetBlockNoNotify(x, y, z, BLOCK_AIR);
	world->NotifyNeighboursWithDelay(x, y, z, 2, TICK_TYPE_NORMAL);

	EntityWorld = world;

	Block = block;

	SetPosition(glm::vec3(x, y - 0.5f, z) + glm::vec3(0.5f));
}

void FallingSandEntity::Update()
{
	Velocity.x *= 0.99f;
	Velocity.z *= 0.99f;
	Velocity.y -= 0.03f;
	Move();

	if (IsGrounded && LastPosition == Position)
	{
		auto state = EntityWorld->GetBlock((int)floor(Position.x), (int)floor(Position.y), (int)floor(Position.z));
		auto b = state.GetDef();
		if (b->IsReplaceable())
		{
			b->BreakAt(EntityWorld, this, (int)floor(Position.x), (int)floor(Position.y), (int)floor(Position.z));
			EntityWorld->SetBlock((int)floor(Position.x), (int)floor(Position.y), (int)floor(Position.z), Block);
		}
		else
		{
			Block.GetDef()->BreakAt(EntityWorld, this, (int)floor(Position.x), (int)floor(Position.y), (int)floor(Position.z));
			EntityWorld->SetBlock((int)floor(Position.x), (int)floor(Position.y), (int)floor(Position.z), state);
		}
		IsDead = true;
	}
}

void FallingSandEntity::WriteToNbt(TagCompound& tag)
{
	BaseEntity::WriteToNbt(tag);
	tag.SetInt("Id", Block.Id);
	tag.SetInt("Metadata", Block.Metadata);
}

void FallingSandEntity::ReadFromNbt(TagCompound& tag)
{
	BaseEntity::ReadFromNbt(tag);
	Block.Id = tag.GetInt("Id");
	Block.Metadata = tag.GetInt("Metadata");
}
