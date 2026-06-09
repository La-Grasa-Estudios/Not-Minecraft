#pragma once

#include "ScreenBase.h"

class ScreenGameOver : public ScreenBase
{
public:
	ScreenGameOver(riDevice* device);
	void Render(glm::vec2 sz) override;
};