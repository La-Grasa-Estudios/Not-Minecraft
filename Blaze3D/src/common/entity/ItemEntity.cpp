#include "ItemEntity.h"

#include "common/Random.h"

#include "common/nbt/NBT.h"
#include "common/World.h"

extern VertexProducer producer;
extern Random g_Random;

DECLARE_ENTITY_CONSTRUCTOR(ItemEntity, [](World* world, TagCompound& tag) {
	return std::make_shared<ItemEntity>(world);
});

ItemEntity::ItemEntity(World* world)
{
	EntityWorld = world;
	BoundingBoxWidth = 0.25f;
	BoundingBoxHeight = 0.25f;
	HeightOffset = 0.25f;
	IsAbleToMakeStepSound = false;
	IsImportant = true;
	m_Player = NULL;
	CanBeRaycasted = false;
	Dimension = world->GetDimensionID();
	IsRemovable = false;
}

ItemEntity::ItemEntity(World* world, glm::vec3 position, const ItemStack& stack) : ItemEntity(world)
{
	SetPosition(position);
	m_Stack = stack;
	m_LiveTime = g_Random.NextFloat() * 10.0f;
	Velocity = glm::vec3{ g_Random.NextFloat() - 0.5f, 0.5f, g_Random.NextFloat() - 0.5f } * 0.25f;
	m_RenderId = g_Random.NextInt(1000000);
}

void ItemEntity::Update()
{
	if (!m_Stack.Id || TicksAlive > 6000)
		ForceRemove = true;
	if (m_PickedByPlayer)
	{
		IsNoclipEnabled = true;
		Velocity = {};
		LastPosition = Position;
		auto target = m_Player->Position;
		target.y -= m_Player->HeightOffset / 2.0f;
		m_AnimationTicks++;
		if (m_AnimationTicks > 3)
		{
			ForceRemove = true;
		}
		Position = glm::mix(m_OrgPosition, target, glm::pow(m_AnimationTicks / 3.0f, 2));
		return;
	}

	bool IsFree = EntityWorld->IsBoundingBoxFree(BoundingBox);

	if (IsFree)
	{
		Velocity.y -= 0.04f;
		Move();
		Velocity *= 0.98f;
	}
	else
	{
		auto point = FindClosestPassableSpace(16, true);
		LastPosition = Position;
		Position = glm::mix(Position, point, 0.2f);
		UpdateAABBBasedOnBBParams();
	}

	Health = 200;

	if (TicksAlive > 20)
	{
		int c;
		auto entities = EntityWorld->GetCollidingEntitiesWith(BoundingBox.grow(0.5f, 0.5f, 0.5f), &c);
		for (int i = 0; i < c; i++)
		{
			auto entity = dynamic_cast<ItemEntity*>(entities[i]);
			if (entity && entity->TicksAlive > 20 && !entity->ForceRemove && entity != this)
			{
				if (entity->m_Stack.Id == m_Stack.Id && entity->m_Stack.Amount <= m_Stack.Amount)
				{
					if (m_Stack.HasSpace(entity->m_Stack.Amount))
					{
						int left = m_Stack.Expand(entity->m_Stack.Amount);
						if (left == 0)
						{
							entity->ForceRemove = true;
						}
						else
						{
							entity->m_Stack.Amount = left;
						}
					}
				}
			}
		}
	}

	if (IsGrounded)
	{
		Velocity *= 0.7f;
	}

	if (IsInLava || IsBurning())
	{
		ForceRemove = true;
		EntityWorld->PlaySound("misc.fizz", Position, 0.15f);
	}
}

void ItemEntity::OnInteractWithPlayer(BaseEntity* other)
{
	if (TicksAlive > 7 && !m_PickedByPlayer)
	{
		auto bef = m_Stack.Amount;
		auto befStack = m_Stack;
		if (other->Inventory.AddStack(m_Stack))
		{
			m_PickedByPlayer = true;
			m_Player = other;
			m_OrgPosition = Position;
			m_Stack = befStack;
			EntityWorld->PlaySound("misc.pop", Position, 0.3f);
		}
		if (bef != m_Stack.Amount)
		{
			EntityWorld->PlaySound("misc.pop", Position, 0.3f);
		}
	}
}

void ItemEntity::WriteToNbt(TagCompound& tag)
{
	BaseEntity::WriteToNbt(tag);
	auto stackTag = m_Stack.WriteToTag();
	stackTag->SetKey("Stack");
	tag.SetTag(stackTag);
	tag.SetTag(std::make_shared<TagFloat>("LiveTime", m_LiveTime));
}

void ItemEntity::ReadFromNbt(TagCompound& tag)
{
	BaseEntity::ReadFromNbt(tag);
	m_Stack.ReadFromTag(tag.GetTagAs<TagCompound>("Stack"));
	m_LiveTime = tag.GetTagAs<TagFloat>("LiveTime");
}