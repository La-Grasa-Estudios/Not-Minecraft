#pragma once

#include "PlayerHandler.h"

class World;

class SPPlayerHandler : public PlayerHandler
{
public:
	SPPlayerHandler(World* world);
	void MainAction(RayCastHit& hit);
	void MainActionDown(RayCastHit& hit, BaseEntity* hitEntity);
	void SecondaryAction(RayCastHit& hit);
	void SecondaryActionDown(RayCastHit& hit, BaseEntity* hitEntity);
	void MainActionUnPressed();
	void SecondaryActionUnPressed();
	void Tick();
	float GetDigProgress();
private:
	World* m_World;
	int m_PlaceCooldown = 0;
	int m_MineCooldown = 0;
	float m_Hardness = 0.0f;
	float m_MineSpeed = 0.0f;
	bool m_IsBreaking = false;
};