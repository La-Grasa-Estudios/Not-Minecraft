#pragma once

#include "EntityRenderer.h"

class FallingSandEntity;

class FallingSandEntityRenderer : public EntityRenderer<FallingSandEntity>
{
public:
	void Render(FallingSandEntity* pEntity, riDevice* pDevice) override;
};