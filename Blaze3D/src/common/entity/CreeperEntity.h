#pragma once

#include "MobEntity.h"

class World;

BEGIN_ENTITY_DECLARATION_WITH_CLASS(CreeperEntity, MobEntity)
public:
	CreeperEntity(World* world);
	CreeperEntity(World* world, glm::vec3 position);

	void OnInteractWithPlayer(BaseEntity* other);
	void Update() override;

protected:

	int m_Fuse = 0;
	int m_CreeperState = 0;

	std::string_view GetCreatureModel() override;
	std::string_view GetCreatureTaunt() override;
	std::string_view GetCreatureHurtSound() override;
	std::string_view GetCreatureDieSound() override;
	int GetCreatureItemDrop() override;

END_ENTITY_DECLARATION