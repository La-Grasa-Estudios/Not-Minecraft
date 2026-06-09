#pragma once

#include "BaseEntity.h"
#include "common/ItemStack.h"

class World;

BEGIN_ENTITY_DECLARATION(ItemEntity)
public:
	ItemEntity(World* world);
	ItemEntity(World* world, glm::vec3 position, const ItemStack& stack);
	void Update() override;
	void OnInteractWithPlayer(BaseEntity* other);
	void WriteToNbt(TagCompound& tag) override;
	void ReadFromNbt(TagCompound& tag) override;
private:
	ItemStack m_Stack;
	ItemStack m_LastSentStack;
	float m_LiveTime = 0.0f;
	bool m_PickedByPlayer = false;
	int m_AnimationTicks = 0;
	int m_RenderId = 0;
	glm::vec3 m_OrgPosition = {};
	BaseEntity* m_Player;
END_ENTITY_DECLARATION