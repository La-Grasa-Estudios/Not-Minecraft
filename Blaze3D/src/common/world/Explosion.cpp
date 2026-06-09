#include "Explosion.h"

#include "common/Random.h"
#include "engine/Audio.h"

#include "common/World.h"
#include "common/entity/BaseEntity.h"

Explosion::Explosion(World* world, uint64_t creator, float x, float y, float z, float power)
{
	m_World = world;
	m_Creator = creator;
	m_PosX = x;
	m_PosY = y;
	m_PosZ = z;
	m_ExplosionPower = power;
}

void Explosion::SetCreateFire()
{
	m_CreateFire = true;
}

void Explosion::CalculateBlocks()
{
	auto creator = m_World->GetEntityByUUID(m_Creator);
	Random random;
	const int searchRadius = 16;
	for (int x = 0; x < searchRadius; x++)
	{
		for (int y = 0; y < searchRadius; y++)
		{
			for (int z = 0; z < searchRadius; z++)
			{
				if (x == 0 || x == searchRadius - 1 || y == 0 || y == searchRadius - 1 || z == 0 || z == searchRadius - 1)
				{
					float xx = ((float)x / ((float)searchRadius - 1.0F) * 2.0F - 1.0F);
					float yy = ((float)y / ((float)searchRadius - 1.0F) * 2.0F - 1.0F);
					float zz = ((float)z / ((float)searchRadius - 1.0F) * 2.0F - 1.0F);
					float length = sqrt(xx * xx + yy * yy + zz * zz);
					xx /= length;
					yy /= length;
					zz /= length;
					float power = m_ExplosionPower * (0.7f + random.NextFloat() * 0.6f);
					float xp = m_PosX;
					float yp = m_PosY;
					float zp = m_PosZ;
					const float decay = 0.3F;
					while (power > 0.0F)
					{
						power -= decay * (12.0F / 16.0F);
						int posX = (int)floor(xp);
						int posY = (int)floor(yp);
						int posZ = (int)floor(zp);
						int id = m_World->GetBlock(posX, posY, posZ).Id;
						if (id > 0)
						{
							power -= (Block::GetBlock(id)->GetBlockResistance(creator) + 0.3f) * decay;
						}

						if (power > 0.0f)
						{
							m_AffectedBlocks.emplace(posX, posY, posZ);
						}

						xp += xx * decay;
						yp += yy * decay;
						zp += zz * decay;
					}
				}
			}
		}
	}

	m_ExplosionPower *= 2.0f;

	float minX = m_PosX - m_ExplosionPower - 1.0f;
	float maxX = m_PosX + m_ExplosionPower + 1.0f;
	float minY = m_PosY - m_ExplosionPower - 1.0f;
	float maxY = m_PosY + m_ExplosionPower + 1.0f;
	float minZ = m_PosZ - m_ExplosionPower - 1.0f;
	float maxZ = m_PosZ + m_ExplosionPower + 1.0f;

	auto position = glm::vec3(m_PosX, m_PosY, m_PosZ);

	AABB bb(minX, minY, minZ, maxX, maxY, maxZ);
	int count;
	auto entities = m_World->GetCollidingEntitiesWith(bb, &count);

	for (int i = 0; i < count; i++)
	{
		auto entity = entities[i];
		float distance = glm::distance(entity->Position, position) / m_ExplosionPower;
		if (distance < 1.0f)
		{
			auto direction = glm::normalize(entity->Position - position);
			float vis = m_World->GetBoundingBoxVisibilityFromPoint(position, entity->BoundingBox);
			float mult = (1.0f - distance) * vis;
			int damage = (int)(glm::pow(mult, 3) / 2.0f * 8.0f * m_ExplosionPower + 1.0f) * 2;
			entity->TakeDamage(creator, damage, DamageSource::EXPLOSION);
			entity->Velocity += direction * vis;
		}
	}
}

void Explosion::ExplodeBlocks()
{
	Random random;
	m_World->PlaySound("random.explode", glm::vec3(m_PosX, m_PosY, m_PosZ), 0.3f);
	auto origin = glm::vec3(m_PosX, m_PosY, m_PosZ);
	for (auto v : m_AffectedBlocks)
	{
		int x = v.x;
		int y = v.y;
		int z = v.z;
		int id = m_World->GetBlock(x, y, z).Id;

		const int particleCount = 1;
		for (int i = 0; i < particleCount; i++)
		{
			auto pos = glm::vec3(x + random.NextFloat(), y + random.NextFloat(), z + random.NextFloat());
			auto distance = pos - origin;
			auto length = glm::length(distance);
			float vMult = 0.5f / (length / m_ExplosionPower + 0.1f);
			vMult *= random.NextFloat() * random.NextFloat() + 0.3f;
			auto vel = distance / length * vMult;
			TagCompound data;
			data.SetFloat("vx", vel.x);
			data.SetFloat("vy", vel.y);
			data.SetFloat("vz", vel.z);
			m_World->SpawnParticle("smoke", pos.x, pos.y, pos.z, &data);
			pos = (pos + origin) / 2.0f;
			m_World->SpawnParticle("explode", pos.x, pos.y, pos.z, &data);
		}

		if (id > 0)
		{
			Block::GetBlock(id)->DropAsItemWithChance(m_World, x, y, z);
			m_World->SetBlock(x, y, z, BLOCK_AIR);
			Block::GetBlock(id)->OnBlockExploded(m_World, x, y, z);
		}
	}
}
