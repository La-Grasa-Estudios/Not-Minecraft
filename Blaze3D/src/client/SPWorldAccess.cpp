#include "SPWorldAccess.h"

#include "engine/Audio.h"

#include "common/nbt/NBT.h"
#include "common/World.h"
#include "renderer/WorldRenderer.h"
#include "Client.h"
#include "ParticleEngine.h"

SPWorldAccess::SPWorldAccess(World* world, WorldRenderer* renderer)
{
	m_Renderer = renderer;
	m_World = world;
}

void SPWorldAccess::SetBlockDirty(int x, int y, int z)
{
	if (m_Renderer)
		m_Renderer->SetDirty(x, y, z);
}

void SPWorldAccess::SetColumnDirty(int x, int z)
{
	if (m_Renderer)
		m_Renderer->SetColumnDirty(x, z);
}

void SPWorldAccess::SetAllDirty()
{
	if (m_Renderer)
		m_Renderer->SetAllDirty();
}

void SPWorldAccess::RenderLoadingScreen(std::string_view title)
{
	Client::GetInstance().RenderLoadingScreen(title);
}

void SPWorldAccess::RenderLoadingScreenWithBar(std::string_view title, float progress)
{
	if (progress > 1.0f) progress = 1.0f;
	Client::GetInstance().RenderLoadingScreenWithBar(title, progress);
}

void SPWorldAccess::PlayAudio(const std::string_view name, const glm::vec3& position, float volume)
{
	AudioEngine::PlaySound(name.data(), position, volume);
}

void SPWorldAccess::PlaySFX(const std::string_view name, float volume)
{
	AudioEngine::PlaySound(name.data(), volume);
}

Particle& SPWorldAccess::SpawnParticle(const std::string_view name, const glm::vec3& position, TagCompound* extraData)
{
	auto& particleEngine = Client::GetInstance().ParticleManager;
	if (name == "explode")
	{
		auto& particle = particleEngine->InitExplosionParticle(position.x, position.y, position.z);
		if (extraData)
		{
			particle.Velocity += glm::vec3(
				extraData->GetFloat("vx"),
				extraData->GetFloat("vy"),
				extraData->GetFloat("vz")
			);
		}
		return particle;
	}
	if (name == "smoke")
	{
		auto& particle = particleEngine->InitSmokeParticle(position.x, position.y, position.z);;
		if (extraData)
		{
			particle.Velocity = glm::vec3(
				extraData->GetFloat("vx"),
				extraData->GetFloat("vy"),
				extraData->GetFloat("vz")
			);
			if (extraData->Contains("r"))
			{
				particle.Color.r = extraData->GetFloat("r");
				particle.Color.g = extraData->GetFloat("g");
				particle.Color.b = extraData->GetFloat("b");
			}
		}
		return particle;
	}
	if (name == "fire")
	{
		return particleEngine->InitTorchFireParticle(position.x, position.y, position.z);
	}
	if (name == "terrain")
	{
		auto& particle = particleEngine->InitTerrainParticle(position.x, position.y, position.z, 0.0f, 0.0f, 0.0f, 0.0f, 0);

		if (extraData)
		{
			particle.Velocity = glm::vec3(
				extraData->GetFloat("vx"),
				extraData->GetFloat("vy"),
				extraData->GetFloat("vz")
			);
			particle.Size += extraData->GetFloat("size");
			particle.RenderData.RenderModeTerrain.BlockId = extraData->GetByte("id");
			particle.Color = glm::vec3(
				extraData->GetFloat("r"),
				extraData->GetFloat("g"),
				extraData->GetFloat("b")
			);
		}

		return particle;
	}
	return particleEngine->InitTerrainParticle(position.x, position.y, position.z, 0.0f, 0.0f, 0.0f, 0.0f, 0);
}
