#pragma once

#include "AnimalEntityBase.h"

class World;

class ChickenEntity : public AnimalEntityBase
{
public:
	ChickenEntity(World* world);
	ChickenEntity(World* world, glm::vec3 position);
	void Update() override;
	bool TakeDamage(BaseEntity* other, int amount, DamageSource source, bool skipImmunity) override;
	//void Render(riDevice* device) override;

protected:

	float m_LastFallDamage;

	std::string_view GetCreatureModel() override;
	std::string_view GetCreatureTaunt() override;
	std::string_view GetCreatureHurtSound() override;
	std::string_view GetCreatureDieSound() override;
	int GetCreatureItemDrop() override;
};