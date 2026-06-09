#pragma once

#include "CreatureEntity.h"

class World;

class MobEntity : public CreatureEntity
{
public:
	MobEntity(World* world);
	MobEntity(World* world, glm::vec3 position);

	void Update() override;
	void Attack(BaseEntity* other, int amt) override;
	bool CanSpawnOn(const glm::vec3& pos) override;
	bool TakeDamage(BaseEntity* other, int amount, DamageSource source, bool skipImmunity) override;

protected:

	virtual void SearchEntityToAttack();

	uint64_t m_TargetEntity = 0;

};