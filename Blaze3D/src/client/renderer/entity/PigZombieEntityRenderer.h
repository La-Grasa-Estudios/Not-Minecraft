#pragma once

#include "CreatureEntityRenderer.h"

class PigZombieEntity;

class PigZombieEntityRenderer : public CreatureEntityRenderer
{
public:
	void OnModelRender(CreatureEntity* pEntity, riDevice* device, const glm::mat4& model) override;
};