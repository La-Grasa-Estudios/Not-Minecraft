#pragma once

#include "EntityRenderer.h"

class TNTPrimedEntity;

class TNTPrimedEntityRenderer : public EntityRenderer<TNTPrimedEntity>
{
public:
	void Render(TNTPrimedEntity* pEntity, riDevice* pDevice) override;
};