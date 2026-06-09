#pragma once

#include "common/IVec3Set.h"

class World;

class Explosion
{
public:
	Explosion(World* world, uint64_t creator, float x, float y, float z, float power);
	void SetCreateFire();
	void CalculateBlocks();
	void ExplodeBlocks();
private:
	IVec3Set m_AffectedBlocks = {};
	float m_PosX = 0, m_PosY = 0, m_PosZ = 0, m_ExplosionPower = 0;
	uint64_t m_Creator = 0;
	bool m_CreateFire = false;
	World* m_World;
};