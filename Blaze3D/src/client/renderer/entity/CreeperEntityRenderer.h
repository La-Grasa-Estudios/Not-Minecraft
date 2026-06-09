#pragma once

#include "CreatureEntityRenderer.h"

class CreeperEntity;

class CreeperEntityRenderer : public CreatureEntityRenderer
{
public:
	void OnModelRender(CreatureEntity* pEntity, riDevice* device, const glm::mat4& model) override;
};