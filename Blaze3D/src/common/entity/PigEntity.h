#pragma once

#include "AnimalEntityBase.h"

class World;

class PigEntity : public AnimalEntityBase
{
public:
	PigEntity(World* world);
	PigEntity(World* world, glm::vec3 position);
	void Update() override;
protected:

	std::string_view GetCreatureModel() override;
	std::string_view GetCreatureTaunt() override;
	std::string_view GetCreatureHurtSound() override;
	std::string_view GetCreatureDieSound() override;
	int GetCreatureItemDrop() override;
};