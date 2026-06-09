#include "CreatureEntity.h"

#include "engine/RenderingInterface.h"
#include "common/Random.h"

#include "common/entity/ItemEntity.h"
#include "common/World.h"

CreatureEntity::CreatureEntity(World* world)
{
	m_MoveSpeed = 0.1f;
	IsAbleToMakeStepSound = true;
	m_CurrentPath.Completed = true;
	StepHeight = 0.5f;
	IsRemovable = false;
	EntityWorld = world;
	Dimension = world->GetDimensionID();
}

CreatureEntity::CreatureEntity(World* world, glm::vec3 position)
{
	SetPosition(position);
}

void CreatureEntity::Update()
{
	Random random;
	m_LastLegSwing = m_LegSwing;
	LastRotation = Rotation;

	m_MoveStrafe *= 0.71f;
	m_MoveForward *= 0.71f;

	int count;
	auto colliding = EntityWorld->GetCollidingEntitiesWith(BoundingBox, &count);

	for (int i = 0; i < count; i++)
	{
		if (colliding[i] != this && colliding[i]->CanBeRaycasted && colliding[i]->CanBeHit)
		{
			PushEntity(colliding[i], 1.0f);
		}
	}

	if (random.NextInt(1000) < m_TauntTimer++)
	{
		m_TauntTimer = -80;
		EntityWorld->PlaySound(GetCreatureTaunt().data(), Position, 0.15f);
	}

	m_InactivityTicks++;
	if (EntityWorld->GetClosestPlayer(Position.x, Position.z, 32.0f) != nullptr)
	{
		m_InactivityTicks = 0;
	}

	//if (EntityWorld->GetClosestPlayer(Position.x, Position.z, 128.0f) == nullptr)
	//{
	//	ForceRemove = true;
	//}

	if (m_InactivityTicks > 600)
	{
		if (random.NextInt(800) == 0)
		{
			ForceRemove = true;
		}
	}

	m_LastDeadTicks = m_DeadTicks;
	if (IsDead)
	{
		m_MoveForward = 0;
		m_MoveStrafe = 0;
		m_DeadTicks++;
		CanBeRaycasted = false;
		if (m_DeadTicks > 20)
		{
			IsRemovable = true;

			int smokeCountX = 5;
			int smokeCountY = 10;

			float stepSizeX = BoundingBoxWidth / (float)smokeCountX;
			float stepSizeY = BoundingBoxHeight / (float)smokeCountY;

			for (float x = BoundingBox.minX; x < BoundingBox.maxX; x += stepSizeX)
			{
				for (float y = BoundingBox.minY; y < BoundingBox.maxY; y += stepSizeY)
				{
					float col = random.NextFloat() * 0.3f + 0.7f;
					TagCompound data;
					data.SetFloat("vx", (random.NextFloat() - 0.5f) * 0.25f);
					data.SetFloat("vz", (random.NextFloat() - 0.5f) * 0.25f);
					data.SetFloat("r", col);
					data.SetFloat("g", col);
					data.SetFloat("b", col);
					auto part = EntityWorld->SpawnParticle("smoke",
						x + random.NextFloat() * stepSizeX,
						y + random.NextFloat() * stepSizeY,
						Position.z + random.NextFloat() * BoundingBoxWidth, &data);
				}
			}

			int id = GetCreatureItemDrop();
			int count = random.NextInt(3);

			for (int i = 0; i < count && id != 0; i++)
			{
				auto entity = std::make_shared<ItemEntity>(EntityWorld, Position + glm::vec3(0.0f, -HeightOffset / 2.0f, 0.0f), ItemStack(id, 1));
				EntityWorld->AddEntity(entity);
			}
		}
	}
	else
	{
		m_DeadTicks = 0;
	}

	if ((m_MoveForward < 0.1f && m_MoveStrafe < 0.1f) || !IsGrounded)
	{
		m_LegSwing -= 0.1f;
	}
	else
	{
		m_LegSwing += 0.1f;
	}

	if (DamageTicks > 0 && !IsGrounded)
	{
		m_LegSwing = 1.0f;
	}

	m_LegSwing = glm::clamp(m_LegSwing, 0.0f, 1.0f);

	float speed = m_MoveSpeed;

	if (IsInWater)
	{
		speed *= 0.6f;
	}

	if (!IsGrounded)
	{
		speed *= 0.4f;
	}

	MoveRelative(m_MoveStrafe, m_MoveForward, speed);

	if (IsGrounded)
	{
		Velocity.x *= 0.6f;
		Velocity.z *= 0.6f;
	}


	if (IsInWater)
	{
		Move();
		Velocity *= glm::vec3(0.80f, 0.81f, 0.80f);
		Velocity.y -= 0.02f;
		if (!IsInLiquid && HorizontalCollision)
		{
			Velocity.y = 0.3f;
		}
	}
	else
	{
		Move();
		Velocity *= glm::vec3(0.91f, 0.98f, 0.91f);
		Velocity.y -= 0.08f;
	}

	if (IsInWater)
	{
		Velocity.y += 0.035f;
	}

	if (m_CurrentPath.Completed && random.NextInt(50) == 0)
	{
		UpdateWanderingPath();
	}

	if (!m_CurrentPath.Completed && !IsDead && random.NextInt(100) != 0)
	{
		FollowPath();
	}

	if (IsInLava)
	{
		TakeDamage(NULL, 4, DamageSource::GENERIC);
		RemainingFireTicks = 300;
	}

	if (RemainingFireTicks > 0)
		RemainingFireTicks--;

	if (IsBurning() && (TicksAlive % 20 == 0 || RemainingFireTicks % 20 == 0))
	{
		TakeDamage(NULL, 1, DamageSource::GENERIC);
	}
}

void CreatureEntity::WriteToNbt(TagCompound& tag)
{
	BaseEntity::WriteToNbt(tag);
}

void CreatureEntity::ReadFromNbt(TagCompound& tag)
{
	BaseEntity::ReadFromNbt(tag);
}

bool CreatureEntity::CanSpawnOn(const glm::vec3& pos)
{
	if (EntityWorld->IsOutsideWorld(floor(pos.x), floor(pos.z)))
		return false;
	auto block = EntityWorld->GetBlock((int)floor(pos.x), (int)floor(pos.y), (int)floor(pos.z)).GetDef();
	return !block->IsSolid() && block->GetLiquidType() == 0 && EntityWorld->GetBlock((int)floor(pos.x), (int)floor(pos.y) - 1, (int)floor(pos.z)).GetDef()->IsSolid() &&
		EntityWorld->GetSkyLight((int)floor(pos.x), (int)floor(pos.y), (int)floor(pos.z)) >= 8 &&
		EntityWorld->GetBlock((int)floor(pos.x), (int)floor(pos.y) - 1, (int)floor(pos.z)).GetDef()->GetId() == BLOCK_GRASS;
}

bool CreatureEntity::TakeDamage(BaseEntity* other, int amount, DamageSource source, bool skipImmunity)
{
	m_InactivityTicks = 0;
	if (BaseEntity::TakeDamage(other, amount, source, skipImmunity))
	{
		if (Health > 0)
			EntityWorld->PlaySound(GetCreatureHurtSound().data(), Position, 0.2f);
		return true;
	}
	return false;
}

void CreatureEntity::OnDeath()
{
	EntityWorld->PlaySound(GetCreatureDieSound().data(), Position, 0.2f);
}

bool CreatureEntity::CanSeeEntity(BaseEntity* other)
{
	glm::vec3 eye = Position;
	glm::vec3 dir = glm::normalize((other->Position + glm::vec3(0.0f, -other->HeightOffset / 3.0f, 0.0f)) - eye);

	return EntityWorld->GetEntityWithRayCast(this, other, eye, dir, 16.0f) != NULL;
}

void CreatureEntity::OnPathReady(NavPath& path)
{
	m_CurrentPath = path;
}

void CreatureEntity::RenderEntityModel(riDevice* device, const glm::mat4& model, const glm::vec4& color, int renderFlags)
{
	
}

void CreatureEntity::FollowPath()
{
	m_PathExpireTicks++;
	if (m_PathExpireTicks > 100 || m_CurrentPath.IsComplete())
	{
		m_CurrentPath.Completed = true;
		m_PathExpireTicks = 0;
		return;
	}

	if (!m_FollowPath)
		return;

	auto next = m_CurrentPath.GetCurrentNode();
	float maxDist = BoundingBoxWidth * 2.0f;

	if (glm::distance(glm::vec2(next.x + 0.5f, next.z + 0.5f), glm::vec2(Position.x, Position.z)) < maxDist)
	{
		m_CurrentPath.IncrementIndex();
		if (m_CurrentPath.IsComplete())
		{
			m_CurrentPath.Completed = true;
			m_PathExpireTicks = 0;
			return;
		}
		else
		{
			next = m_CurrentPath.GetCurrentNode();
		}
	}

	int floorY = (int)floor(BoundingBox.minY + 0.5f);

	float dx = (next.x + (int)(BoundingBoxWidth + 1.0f) * 0.5f) - Position.x;
	float dz = (next.z + (int)(BoundingBoxWidth + 1.0f) * 0.5f) - Position.z;
	float dy = next.y - floorY;

	float angle = glm::atan(dz, dx) * (180.0f / glm::pi<float>()) - 90.0f;
	float diff = angle - Rotation.y;

	while (diff < -180.0f) diff += 360.0f;
	while (diff >= 180.0f) diff -= 360.0f;
	diff = glm::clamp(diff, -30.0f, 30.0f);
	Rotation.y += diff * 0.5f;

	m_MoveForward = 1.0f;

	if (dy > 0.0f && IsGrounded)
	{
		Velocity.y = 0.4f;
	}

	if (dy < 0.0f && !IsGrounded)
	{
		m_MoveForward = 0.0f;
	}
}

void CreatureEntity::UpdateWanderingPath()
{
	Random random;
	const int radius = 12;
	int x = (int)floor(Position.x);
	int y = (int)floor(Position.y);
	int z = (int)floor(Position.z);
	for (int i = 0; i < 100; i++)
	{
		int xx = x + random.NextInt(radius * 2) - radius;
		int yy = y + 5 - random.NextInt(10);
		int zz = z + random.NextInt(radius * 2) - radius;
		if (CanSpawnOn(glm::vec3(xx, yy, zz)))
		{
			EntityWorld->RequestPathForEntity(this, glm::ivec3((int)floor(Position.x), (int)floor(Position.y - HeightOffset + 0.5f), (int)floor(Position.z)), glm::ivec3(xx, yy, zz));
			break;
		}
	}
}
