#pragma once

#include <thirdparty/glm/ext.hpp>
#include <string_view>

class TagCompound;
class World;
struct Particle;

class IWorldAccess
{
public:
	virtual void OnBlockChanged(int x, int y, int z) {};
	virtual void SetBlockDirty(int x, int y, int z) = 0;
	virtual void SetColumnDirty(int x, int z) = 0;
	virtual void SetAllDirty() = 0;
	virtual void RenderLoadingScreen(std::string_view title) = 0;
	virtual void RenderLoadingScreenWithBar(std::string_view title, float progress) = 0;
	virtual void PlayAudio(const std::string_view name, const glm::vec3& position, float volume) = 0;
	virtual void PlaySFX(const std::string_view name, float volume) = 0;
	virtual void OnEntitySpawned(uint64_t uuid) {};
	virtual void OnEntityDestroy(uint64_t uuid) {};
	virtual Particle& SpawnParticle(const std::string_view name, const glm::vec3& position, TagCompound* extraData) = 0;
};