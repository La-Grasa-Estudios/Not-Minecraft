#pragma once

#include "MobEntity.h"

class World;

class ZombieEntity : public MobEntity
{
public:
	ZombieEntity(World* world);
	ZombieEntity(World* world, glm::vec3 position);

	void OnInteractWithPlayer(BaseEntity* other);

protected:

	std::string_view GetCreatureModel() override;
	std::string_view GetCreatureTaunt() override;
	std::string_view GetCreatureHurtSound() override;
	std::string_view GetCreatureDieSound() override;
	int GetCreatureItemDrop() override;

};