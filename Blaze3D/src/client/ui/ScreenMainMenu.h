#pragma once

#include "ScreenBase.h"

class ScreenMainMenu : public ScreenBase
{
public:
	ScreenMainMenu(riDevice* device);
	void Render(glm::vec2 sz) override;
};