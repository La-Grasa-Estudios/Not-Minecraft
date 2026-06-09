#pragma once

#include "AnimalEntityBase.h"

class World;

class CowEntity : public AnimalEntityBase
{
public:
	CowEntity(World* world);
	CowEntity(World* world, glm::vec3 position);
	
protected:

	std::string_view GetCreatureModel() override;
	std::string_view GetCreatureTaunt() override;
	std::string_view GetCreatureHurtSound() override;
	std::string_view GetCreatureDieSound() override;
	int GetCreatureItemDrop() override;
};