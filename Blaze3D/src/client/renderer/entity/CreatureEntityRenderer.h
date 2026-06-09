#pragma once

#include <thirdparty/glm/ext.hpp>

#include "EntityRenderer.h"

constexpr int CREATURE_RENDER_FLAG_NOTHING = 0;
constexpr int CREATURE_RENDER_FLAG_TEXTURING = 1 << 0;
constexpr int CREATURE_RENDER_FLAG_WORLD_LIGHTING = 1 << 1;
constexpr int CREATURE_RENDER_FLAG_LIGHTING = 1 << 2;

class CreatureEntity;

class CreatureEntityRenderer : public EntityRenderer<CreatureEntity>
{
public:
	void Render(CreatureEntity* pEntity, riDevice* pDevice) override;
	void RenderEntityModel(CreatureEntity* pEntity, riDevice* device, const glm::mat4& model, const glm::vec4& color, int renderFlags);
	template<typename T>
	T* GetEntity(CreatureEntity* entity)
	{
		return static_cast<T*>(entity);
	}
	virtual void OnModelRender(CreatureEntity* pEntity, riDevice* device, const glm::mat4& model) {};
};