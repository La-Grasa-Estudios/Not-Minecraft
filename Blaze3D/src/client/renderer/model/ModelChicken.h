#pragma once

#include "ModelBox.h"
#include "ModelBase.h"

class ModelChicken : public ModelBase
{
public:
	ModelChicken();

	void Render(VertexProducer& producer, float walkingProgress, float digProgress, float idleProgress, float armsAngle, float legSwingFactor, bool applyWalkToArms) override;
	ModelBox Body;
	ModelBox WingLeft;
	ModelBox WingRight;
	ModelBox LegLeft;
	ModelBox LegRight;
	ModelBox ClawLeft;
	ModelBox ClawRight;
	ModelBox Head;
	ModelBox Beak;
	ModelBox Wattle;
};