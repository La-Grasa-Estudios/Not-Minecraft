#include "ParticleEngine.h"

#include "engine/memory/MemoryAllocator.h"
#include "engine/System.h"
#include "engine/Time.h"
#include "engine/RenderingInterface.h"
#include "common/Random.h"

#include "client/Client.h"
#include "client/ResourceManager.h"
#include "common/entity/AABB.h"
#include "common/entity/PlayerEntity.h"
#include "common/World.h"
#include "client/renderer/VertexProducer.h"
#include "common/block/Block.h"

#include <array>
#include <functional>

constexpr size_t PARTICLE_CAP = 3000;
extern VertexProducer producer;
extern Random g_Random;

static std::array<std::function<void(Particle&, World*)>, 32> g_TickFunctionPtrs;
static std::array<std::function<void(Particle&, World*)>, 32> g_UpdateFunctionPtrs;
static std::array<std::function<void(Particle&, World*, VertexProducer&, float, float, float, float, float)>, 32> g_RenderFunctionPtrs;

static void TickParticleModeTerrain(Particle& part, World* world)
{
	part.Velocity.y -= 0.04f;
	part.Velocity.x *= 0.98f;
	part.Velocity.y *= 0.98f;
	part.Velocity.z *= 0.98f;

	if (part.IsGrounded)
	{
		part.Velocity.x *= 0.4f;
		part.Velocity.z *= 0.4f;
	}

	float size = part.Size / 2.0f * 0.1f;

	int count = 32;
	AABB bb = {
		part.Position.x - size, part.Position.y - size, part.Position.z - size,
		part.Position.x + size, part.Position.y + size, part.Position.z + size
	};

	AABB boxes[32];
	world->GetBoxes(bb.expand(part.Velocity.x, part.Velocity.y, part.Velocity.z), boxes, count);

	float ya = part.Velocity.y;

	for (int i = 0; i < count; i++) {
		part.Velocity.y = boxes[i].clipYCollide(bb, part.Velocity.y);
	}

	bb.move(0, part.Velocity.y, 0);

	for (int i = 0; i < count; i++) {
		part.Velocity.x = boxes[i].clipXCollide(bb, part.Velocity.x);
	}

	bb.move(part.Velocity.x, 0, 0);

	for (int i = 0; i < count; i++) {
		part.Velocity.z = boxes[i].clipZCollide(bb, part.Velocity.z);
	}

	bb.move(0.0f, 0.0f, part.Velocity.z);

	part.LastPosition = part.Position;
	part.Position.x = (float)(bb.minX + bb.maxX) / 2.0f;
	part.Position.y = (float)(bb.minY + bb.maxY) / 2.0f;
	part.Position.z = (float)(bb.minZ + bb.maxZ) / 2.0f;

	part.IsGrounded = ya != part.Velocity.y && ya < 0.0;;

	part.TimeToLive -= 0.05f;

	if (part.TimeToLive < 0.0f)
	{
		part.IsAlive = false;
	}
}

static void RenderParticleModeTerrain(Particle& part, World* world, VertexProducer& producer, float xa, float xa2, float ya, float za, float za2)
{
	int tex = Block::GetBlock(part.RenderData.RenderModeTerrain.BlockId)->GetTextureForFace(part.RenderData.RenderModeTerrain.Face, NULL, 0, 0, 0);
	float xt = ((tex % 16) * 16.0f) + part.RenderData.RenderModeTerrain.UvFracX;
	float yt = ((tex / 16) * 16.0f) + part.RenderData.RenderModeTerrain.UvFracY;
	float u0 = xt / 256.0f;
	float u2 = (xt + 4.0f) / 256.0f;
	float v0 = yt / 256.0f;
	float v2 = (yt + 4.0f) / 256.0f;
	float r = 0.1f * part.Size;

	producer.SetColor(part.Color.r, part.Color.g, part.Color.b);
	producer.AddVertex(0.0f - xa * r - xa2 * r, 0.0f - ya * r, 0.0f - za * r - za2 * r, u0, v2);
	producer.AddVertex(0.0f - xa * r + xa2 * r, 0.0f + ya * r, 0.0f - za * r + za2 * r, u0, v0);
	producer.AddVertex(0.0f + xa * r + xa2 * r, 0.0f + ya * r, 0.0f + za * r + za2 * r, u2, v0);
	producer.AddVertex(0.0f + xa * r - xa2 * r, 0.0f - ya * r, 0.0f + za * r - za2 * r, u2, v2);
}

static void TickParticleModeTorch(Particle& part, World* world)
{
	part.LastPosition = part.Position;

	part.Size -= 0.05f;

	if (part.Size < 0.0f)
	{
		part.IsAlive = false;
	}
}

static void RenderParticleModeTorch(Particle& part, World* world, VertexProducer& producer, float xa, float xa2, float ya, float za, float za2)
{
	int tex = part.RenderData.RenderModeItem.ItemId;
	float xt = ((tex % 16) * 8.0f);
	float yt = ((tex / 16) * 8.0f);
	float u0 = xt / 128.0f;
	float u2 = (xt + 8.0f) / 128.0f;
	float v0 = yt / 128.0f;
	float v2 = (yt + 8.0f) / 128.0f;
	float r = 0.1f * part.Size;

	producer.SetColor(part.Color.r, part.Color.g, part.Color.b);
	producer.AddVertex(0.0f - xa * r - xa2 * r, 0.0f - ya * r, 0.0f - za * r - za2 * r, u0, v2);
	producer.AddVertex(0.0f - xa * r + xa2 * r, 0.0f + ya * r, 0.0f - za * r + za2 * r, u0, v0);
	producer.AddVertex(0.0f + xa * r + xa2 * r, 0.0f + ya * r, 0.0f + za * r + za2 * r, u2, v0);
	producer.AddVertex(0.0f + xa * r - xa2 * r, 0.0f - ya * r, 0.0f + za * r - za2 * r, u2, v2);
}

static void TickParticleModeSmoke(Particle& part, World* world)
{
	part.Velocity.y += 0.004f;
	part.LastPosition = part.Position;
	part.Position += part.Velocity;
	part.Velocity *= 0.96f;

	part.Age++;
	part.RenderData.RenderModeItem.ItemId = 7 - (part.Age << 3) / part.LifeTimeInTicks;

	if (part.Age >= part.LifeTimeInTicks)
	{
		part.IsAlive = false;
	}
}

static void TickParticleModeExplosion(Particle& part, World* world)
{
	part.Velocity.y += 0.004f;
	part.LastPosition = part.Position;
	part.Position += part.Velocity;
	part.Velocity *= 0.9f;

	part.Age++;
	part.RenderData.RenderModeItem.ItemId = 7 - (part.Age * 8) / part.LifeTimeInTicks;

	if (part.Age >= part.LifeTimeInTicks)
	{
		part.IsAlive = false;
	}
}

ParticleEngine::ParticleEngine(World* world)
{
	m_Particles = new Particle[PARTICLE_CAP];
	m_World = world;
	g_TickFunctionPtrs[PARTICLE_RENDER_MODE_TERRAIN] = TickParticleModeTerrain;
	g_RenderFunctionPtrs[PARTICLE_RENDER_MODE_TERRAIN] = RenderParticleModeTerrain;

	g_TickFunctionPtrs[PARTICLE_RENDER_MODE_TORCH] = TickParticleModeTorch;
	g_RenderFunctionPtrs[PARTICLE_RENDER_MODE_TORCH] = RenderParticleModeTorch;

	g_TickFunctionPtrs[PARTICLE_RENDER_MODE_SMOKE] = TickParticleModeSmoke;
	g_RenderFunctionPtrs[PARTICLE_RENDER_MODE_SMOKE] = RenderParticleModeTorch;

	g_TickFunctionPtrs[PARTICLE_RENDER_MODE_EXPLOSION] = TickParticleModeExplosion;
	g_RenderFunctionPtrs[PARTICLE_RENDER_MODE_EXPLOSION] = RenderParticleModeTorch;
}

ParticleEngine::~ParticleEngine()
{
	delete[] m_Particles;
}

Particle& ParticleEngine::AddParticle(Particle& particle)
{
	particle.IsAlive = true;
	particle.LastPosition = particle.Position;
	auto index = m_SearchStartIndex;
	m_Particles[index] = particle;
	m_SearchStartIndex++;
	if (m_SearchStartIndex >= PARTICLE_CAP - 1)
	{
		m_SearchStartIndex = 0;
	}
	return m_Particles[index];
}

void ParticleEngine::Update()
{
	for (size_t i = 0; i < PARTICLE_CAP; i++)
	{
		auto& part = m_Particles[i];
		if (part.IsAlive)
		{
			if (g_UpdateFunctionPtrs[part.RenderMode])
			{
				g_UpdateFunctionPtrs[part.RenderMode](part, m_World);
			}
		}
	}
}

void ParticleEngine::Tick()
{
	for (size_t i = 0; i < PARTICLE_CAP; i++)
	{
		auto& part = m_Particles[i];
		if (part.IsAlive)
		{
			if (g_TickFunctionPtrs[part.RenderMode])
			{
				g_TickFunctionPtrs[part.RenderMode](part, m_World);
			}
		}
	}
}

void ParticleEngine::Render(riDevice* device)
{
	auto& client = Client::GetInstance();
	auto player = client.Player.get();
	float xa = -(float)glm::cos(player->Rotation.y * 3.141592653589793 / 180.0);
	float za = -(float)glm::sin(player->Rotation.y * 3.141592653589793 / 180.0);
	float xa2 = -za * (float)glm::sin(player->Rotation.x * 3.141592653589793 / 180.0);
	float za2 = xa * (float)glm::sin(player->Rotation.x * 3.141592653589793 / 180.0);
	float ya = (float)glm::cos(player->Rotation.x * 3.141592653589793 / 180.0);

	device->SetTexturing(true);
	device->SetCullMode(RI_CULL_MODE_NONE);
	for (size_t kind = 0; kind < PARTICLE_RENDER_MODE_COUNT; kind++)
	{
		producer.Reset();
		producer.SetColor(0xFFFFFFFF);
		for (size_t i = 0; i < PARTICLE_CAP; i++)
		{
			auto& part = m_Particles[i];
			if (part.IsAlive && part.RenderMode == kind)
			{
				glm::vec3 Position = glm::mix(part.LastPosition, part.Position, client.UpdateTimer.a);
				auto PositionFloored = glm::ivec3(glm::floor(Position));

				producer.SetLightMultiplier(m_World->LightLUT[m_World->GetMixedLight(PositionFloored.x, PositionFloored.y, PositionFloored.z)]);
				producer.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

				producer.SetTranslation(Position.x, Position.y, Position.z);

				g_RenderFunctionPtrs[kind](part, m_World, producer, xa, xa2, ya, za, za2);
			}
		}
		switch (kind)
		{
		case PARTICLE_RENDER_MODE_NORMAL:
		case PARTICLE_RENDER_MODE_TERRAIN:
			client.pResourceManager->BindTexture(E_TextureResource_Terrain, device);
			break;
		case PARTICLE_RENDER_MODE_SMOKE:
		case PARTICLE_RENDER_MODE_TORCH:
			client.pResourceManager->BindTexture(E_TextureResource_Particles, device);
			break;
		default:
			continue;
		}
		device->Draw(producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, producer.GetVertexCount());
	}
	device->SetCullMode(RI_CULL_MODE_BACK);
	device->SetTexturing(false);

}

Particle& ParticleEngine::InitTerrainParticle(float x, float y, float z, float velX, float velY, float velZ, float size, int blockId)
{
	Particle part{};

	part.Color = glm::vec3(1.0f);
	part.Position = glm::vec3(x, y, z);
	part.Velocity = glm::vec3(velX, velY, velZ);
	part.Velocity = part.Velocity * glm::vec3(g_Random.NextFloat() + 1.0) * 0.15f;
	part.TimeToLive = 1.0f + g_Random.NextFloat() * 2.0f;
	part.Size = size + g_Random.NextFloat() * 0.4f;
	part.RenderData.RenderModeTerrain.BlockId = blockId;
	part.RenderData.RenderModeTerrain.Face = 0;
	part.RenderData.RenderModeTerrain.UvFracX = 8.0f + g_Random.NextFloat() * 8.0f - 4.0f;
	part.RenderData.RenderModeTerrain.UvFracY = 8.0f + g_Random.NextFloat() * 8.0f - 4.0f;
	part.RenderMode = PARTICLE_RENDER_MODE_TERRAIN;

	return AddParticle(part);
}

Particle& ParticleEngine::InitTorchFireParticle(float x, float y, float z)
{
	Particle part{};
	part.Position = glm::vec3(x, y, z);
	part.Size = 2.0f;
	part.RenderMode = PARTICLE_RENDER_MODE_TORCH;
	part.RenderData.RenderModeItem.ItemId = 48;
	part.Color = glm::vec3(1.0f);

	if (g_Random.NextInt(3) == 0)
	{
		InitSmokeParticle(x, y, z);
	}

	return AddParticle(part);
}

Particle& ParticleEngine::InitSmokeParticle(float x, float y, float z)
{
	Particle part{};
	part.Position = glm::vec3(x, y, z);
	part.Size = 2.0f;
	part.RenderMode = PARTICLE_RENDER_MODE_SMOKE;
	part.RenderData.RenderModeItem.ItemId = 7;
	part.LifeTimeInTicks = (int)(8.0 / (g_Random.NextFloat() * 0.8f + 0.2f));
	part.Color = glm::vec3((g_Random.NextFloat() * 0.3f));
	return AddParticle(part);
}

Particle& ParticleEngine::InitExplosionParticle(float x, float y, float z)
{
	Particle part{};
	part.Position = glm::vec3(x, y, z);
	part.Size = g_Random.NextFloat() * g_Random.NextFloat() * 6.0f + 1.0f;
	part.RenderMode = PARTICLE_RENDER_MODE_SMOKE;
	part.RenderData.RenderModeItem.ItemId = 7;
	part.LifeTimeInTicks = (int)(16.0 / (g_Random.NextFloat() * 0.8f + 0.2f)) + 2;
	part.Color = glm::vec3(g_Random.NextFloat() * 0.3f + 0.7f);
	part.Velocity.x = (g_Random.NextFloat() * 2.0f - 1.0f) * 0.05f;
	part.Velocity.y = (g_Random.NextFloat() * 2.0f - 1.0f) * 0.05f;
	part.Velocity.z = (g_Random.NextFloat() * 2.0f - 1.0f) * 0.05f;
	return AddParticle(part);
}
