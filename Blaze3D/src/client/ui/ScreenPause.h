#pragma once

#include "ScreenBase.h"

class ScreenPause : public ScreenBase
{
public:
	ScreenPause(riDevice* device);
	void Render(glm::vec2 sz) override;
	bool DoesGuiPauseGame() override;
};