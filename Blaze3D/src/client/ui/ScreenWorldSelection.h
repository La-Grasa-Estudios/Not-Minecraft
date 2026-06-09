#pragma once

#include "ScreenBase.h"

class ScreenWorldSelection : public ScreenBase
{
public:
	ScreenWorldSelection(riDevice* device);
	void Render(glm::vec2 sz) override;
};