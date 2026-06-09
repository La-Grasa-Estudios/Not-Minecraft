#pragma once

#include "MobEntity.h"

class World;

BEGIN_ENTITY_DECLARATION_WITH_CLASS(PigZombieEntity, MobEntity)
public:
	PigZombieEntity(World* world);
	PigZombieEntity(World* world, glm::vec3 position);

	void OnInteractWithPlayer(BaseEntity* other);
	bool TakeDamage(BaseEntity* other, int amount, DamageSource source, bool skipImmunity) override;
	bool CanSpawnOn(const glm::vec3& pos) override;
	void Update() override;

protected:

	void BecomeAngry(BaseEntity* target);
	void SearchEntityToAttack() override;

	int m_AngerLevel = 0;
	int m_AngrySoundDelay = 0;

	std::string_view GetCreatureModel() override;
	std::string_view GetCreatureTaunt() override;
	std::string_view GetCreatureHurtSound() override;
	std::string_view GetCreatureDieSound() override;
	int GetCreatureItemDrop() override;
END_ENTITY_DECLARATION