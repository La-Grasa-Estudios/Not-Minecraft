#pragma once

#include "ModelBase.h"
#include "ModelPartItem.h"
#include "ModelBox.h"

class VertexProducer;

class ModelHumanoid : public ModelBase
{
public:
	ModelHumanoid();

	void Render(VertexProducer& producer, float walkingProgress, float digProgress, float idleProgress, float armsAngle, float legSwingFactor, bool applyWalkToArms) override;
	ModelBox Torso;
	ModelBox ArmLeft;
	ModelBox ArmRight;
	ModelBox LegLeft;
	ModelBox LegRight;
	ModelBox Head;
	ModelPartItem HeldItem;
	ModelPartItem HeldItemBlock;
	ModelPartItem HeldItemBlockSprite;
	ModelPartItem HeldItemNonTool;
	ModelPartItem HeldItemBow;
};