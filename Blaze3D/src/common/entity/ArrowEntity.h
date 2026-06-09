#pragma once

#include "BaseEntity.h"

class World;

BEGIN_ENTITY_DECLARATION(ArrowEntity)
public:
	ArrowEntity(World* world);
	ArrowEntity(World* world, BaseEntity* owner, glm::vec3 position);

	void Update() override;
	void SetHeading(float vx, float vy, float vz, float force, float spread);
	bool TakeDamage(BaseEntity* other, int amount, DamageSource source, bool skipImmunity) override;
	void OnInteractWithPlayer(BaseEntity* other);

	void WriteToNbt(TagCompound& tag) override;
	void ReadFromNbt(TagCompound& tag) override;

protected:

	glm::vec3 HeadingBeforeHit = {};

	bool HasHit = false;

	bool m_DoesArrowBelongToPlayer = false;
	float m_RenderArrowShake = 0.0f;
	int m_ArrowShake = 0;
	int m_SentArrowShake = 0;
	uint64_t m_Owner;
END_ENTITY_DECLARATION