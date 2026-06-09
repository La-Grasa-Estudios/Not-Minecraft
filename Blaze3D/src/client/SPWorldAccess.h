#pragma once

#include "common/IWorldAccess.h"

class WorldRenderer;

class SPWorldAccess : public IWorldAccess
{
public:
	SPWorldAccess(World* world, WorldRenderer* renderer);
	void SetBlockDirty(int x, int y, int z);
	void SetColumnDirty(int x, int z);
	void SetAllDirty();

	void RenderLoadingScreen(std::string_view title);
	void RenderLoadingScreenWithBar(std::string_view title, float progress);

	void PlayAudio(const std::string_view name, const glm::vec3& position, float volume);
	void PlaySFX(const std::string_view name, float volume);
	Particle& SpawnParticle(const std::string_view name, const glm::vec3& position, TagCompound* extraData);
private:
	WorldRenderer* m_Renderer;
	World* m_World;
};