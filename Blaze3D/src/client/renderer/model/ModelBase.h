#pragma once

class VertexProducer;

class ModelBase
{
public:
	virtual void Render(VertexProducer& producer, float walkingProgress, float digProgress, float idleProgress, float armsAngle, float legSwingFactor, bool applyWalkToArms) {};
	void SetParameter(int index, float param);
	void* Texture;
	float Parameters[64];
};