#include "ArrowEntity.h"

#include "common/Random.h"

#include "common/World.h"
#include "common/nbt/NBT.h"
#include "common/item/Item.h"

DECLARE_ENTITY_CONSTRUCTOR(ArrowEntity, [](World* world, TagCompound& tag) {
	return std::make_shared<ArrowEntity>(world);
});

ArrowEntity::ArrowEntity(World* world)
{
	AllowSliding = false;
	BoundingBoxWidth = 0.25f;
	BoundingBoxHeight = 0.25f;
	HeightOffset = 0.0f;
	StepHeight = 0.0f;
	m_Owner = 0;
	m_ArrowShake = 0;
	m_DoesArrowBelongToPlayer = false;
	EntityWorld = world;
	CanBeHit = false;
	CanBeRaycasted = false;
	IsRemovable = false;
	IsAbleToMakeStepSound = false;
	HeadingBeforeHit = {};
}

ArrowEntity::ArrowEntity(World* world, BaseEntity* owner, glm::vec3 position) : ArrowEntity(world)
{
	SetPosition(position);
	if (owner)
	{
		m_Owner = owner->GetUUID();
	}
	m_DoesArrowBelongToPlayer = world->GetClosestPlayer(Position.x, Position.z, 1.0f) == owner;
}

void ArrowEntity::Update()
{
	LastRotation = Rotation;
	bool wasGrounded = IsGrounded;
	bool hadHorizontalCollision = HorizontalCollision;
	m_RenderArrowShake = m_ArrowShake;

	if (m_ArrowShake > 0)
	{
		m_ArrowShake--;
	}

	int count;
	auto entities = EntityWorld->GetCollidingEntitiesWith(BoundingBox.expand(Velocity.x, Velocity.y, Velocity.z), &count);
	for (int i = 0; i < count; i++)
	{
		auto entity = entities[i];
		if (entity && entity != this)
		{
			if ((entity->GetUUID() != m_Owner || TicksAlive > 3) && !IsGrounded && entity->CanBeHit && entity->CanBeRaycasted)
			{
				auto owner = EntityWorld->GetEntityByUUID(m_Owner);
				if (!owner)
				{
					owner = this;
				}
				if (entity->TakeDamage(owner, 4, DamageSource::MOB))
				{
					ForceRemove = true;
					entity->Velocity += Velocity * 0.2f;
					entity->Velocity.y = 0.2f;
					//EntityWorld->PlaySoundEffect("random.drr", 0.1f);
					EntityWorld->PlaySound("random.drr", Position, 0.3f);
				}
				else
				{
					Velocity *= -0.1f;
				}
			}
		}
	}

	if (IsGrounded)
	{
		Velocity.x *= Random::GlobalRand.NextFloat() * 0.2f;
		Velocity.y *= Random::GlobalRand.NextFloat() * 0.2f;
		Velocity.z *= Random::GlobalRand.NextFloat() * 0.2f;
	}

	RayCastHit hitInfo;

	bool hit = EntityWorld->RayCast(Position, glm::normalize(Velocity), glm::length(Velocity), hitInfo);

	if (hit)
	{
		auto v = glm::normalize(Velocity);

		if (!HasHit)
		{
			HasHit = true;
			HeadingBeforeHit = Velocity;
		}

		Velocity = hitInfo.HitPosition - Position;
		float length = glm::length(Velocity);
		//Position -= Velocity / length * 0.05f;
		IsGrounded = true;
	}
	else
	{
		HasHit = false;
		IsGrounded = false;
		HeadingBeforeHit = Velocity;
	}

	TicksAlive++;

	LastPosition = Position;
	Position += Velocity;

	auto pos = Position;
	UpdateAABBBasedOnBBParams();
	Position = pos;

	Velocity *= 0.98f;
	Velocity.y -= 0.03f;

	if (IsGrounded)
	{
		Velocity.x *= 0.7f;
		Velocity.z *= 0.7f;
	}
	else if (glm::length(Velocity) > 0.1f)
	{
		float vx = Velocity.x, vy = Velocity.y, vz = Velocity.z;
		float l = glm::length(glm::vec2(vx, vz));
		float angleZ = glm::atan(vy, l) * 180.0f / glm::pi<float>();
		float angleY = glm::atan(vx, vz) * 180.0f / glm::pi<float>();
		Rotation.z = angleZ;
		Rotation.y = angleY;
	}

	if ((wasGrounded != IsGrounded && IsGrounded) || (hadHorizontalCollision != HorizontalCollision))
	{
		m_ArrowShake = 10;
		EntityWorld->PlaySound("random.drr", Position, 0.3f);
	}

	if (TicksAlive > 1200)	
	{
		ForceRemove = true;
	}
}

void ArrowEntity::SetHeading(float vx, float vy, float vz, float force, float spread)
{
	Random random;
	float length = glm::length(glm::vec3(vx, vy, vz));
	vx /= length;
	vy /= length;
	vz /= length;
	float deviation = 0.0075f;
	vx += (float)random.NextGaussian() * deviation * spread;
	vy += (float)random.NextGaussian() * deviation * spread;
	vz += (float)random.NextGaussian() * deviation * spread;
	vx *= force;
	vy *= force;
	vz *= force;
	Velocity = { vx, vy, vz };
	float l = glm::length(glm::vec2(vx, vz));
	float angleX = glm::atan(vy, l) * 180.0f / glm::pi<float>();
	float angleY = glm::atan(vx, vz) * 180.0f / glm::pi<float>();
	Rotation.z = angleX;
	Rotation.y = angleY;
}

bool ArrowEntity::TakeDamage(BaseEntity* other, int amount, DamageSource source, bool skipImmunity)
{
	return false;
}

void ArrowEntity::OnInteractWithPlayer(BaseEntity* other)
{
	if (m_DoesArrowBelongToPlayer && TicksAlive > 3 && IsGrounded)
	{
		ItemStack stack{ Item::Arrow->GetId(), 1 };
		other->Inventory.AddStack(stack);
		ForceRemove = true;
		EntityWorld->PlaySoundEffect("misc.pop", 0.3f);
	}
}

void ArrowEntity::WriteToNbt(TagCompound& tag)
{
	BaseEntity::WriteToNbt(tag);
	tag.SetFloat("HeadingX", HeadingBeforeHit.x);
	tag.SetFloat("HeadingY", HeadingBeforeHit.y);
	tag.SetFloat("HeadingZ", HeadingBeforeHit.z);
	tag.SetBool("HasHit", HasHit);
	tag.SetLong("Owner", m_Owner);
}

void ArrowEntity::ReadFromNbt(TagCompound& tag)
{
	BaseEntity::ReadFromNbt(tag);
	HeadingBeforeHit.x = tag.GetFloat("HeadingX");
	HeadingBeforeHit.y = tag.GetFloat("HeadingY");
	HeadingBeforeHit.z = tag.GetFloat("HeadingZ");
	HasHit = tag.GetBool("HasHit");
	m_Owner = tag.GetLong("Owner");
}