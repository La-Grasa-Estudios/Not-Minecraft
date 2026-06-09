#pragma once

#include "common/World.h"

class PlayerHandler
{
public:
	virtual void MainAction(RayCastHit& hit) = 0;
	virtual void MainActionDown(RayCastHit& hit, BaseEntity* hitEntity) = 0;
	virtual void SecondaryAction(RayCastHit& hit) = 0;
	virtual void SecondaryActionDown(RayCastHit& hit, BaseEntity* hitEntity) = 0;
	virtual void MainActionUnPressed() = 0;
	virtual void SecondaryActionUnPressed() = 0;
	virtual void Tick() = 0;
	virtual float GetDigProgress() = 0;
};