#pragma once

#include "EntityRenderer.h"

class ArrowEntity;

class ArrowEntityRenderer : public EntityRenderer<ArrowEntity>
{
public:
	void Render(ArrowEntity* pEntity, riDevice* pDevice) override;
};