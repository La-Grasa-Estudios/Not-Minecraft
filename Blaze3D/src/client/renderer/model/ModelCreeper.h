#pragma once

#include "ModelBox.h"
#include "ModelBase.h"

class ModelCreeper : public ModelBase
{
public:
	ModelCreeper();

	void Render(VertexProducer& producer, float walkingProgress, float digProgress, float idleProgress, float armsAngle, float legSwingFactor, bool applyWalkToArms) override;
	ModelBox Body;
	ModelBox ArmLeft;
	ModelBox ArmRight;
	ModelBox LegLeft;
	ModelBox LegRight;
	ModelBox Head;
};