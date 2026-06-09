#pragma once

#include "BaseEntity.h"

BEGIN_ENTITY_DECLARATION(CreatureEntity)
public:

	CreatureEntity(World* world);
	CreatureEntity(World* world, glm::vec3 position);

	void Update() override;
	void WriteToNbt(TagCompound& tag) override;
	void ReadFromNbt(TagCompound& tag) override;
	virtual bool CanSpawnOn(const glm::vec3& pos);
	bool TakeDamage(BaseEntity* other, int amount, DamageSource source, bool skipImmunity = false) override;
	void OnDeath() override;
	bool CanSeeEntity(BaseEntity* other);

	void OnPathReady(NavPath& path) override;

	bool RequiresNetworkUpdates();

protected:

	void RenderEntityModel(riDevice* device, const glm::mat4& model, const glm::vec4& color, int renderFlags);

	virtual std::string_view GetCreatureModel() = 0;
	virtual std::string_view GetCreatureTaunt() = 0;
	virtual std::string_view GetCreatureHurtSound() = 0;
	virtual std::string_view GetCreatureDieSound() = 0;
	virtual int GetCreatureItemDrop() { return 0; };

	void FollowPath();
	void UpdateWanderingPath();

	float m_MoveForward = 0.0f;
	float m_MoveStrafe = 0.0f;
	float m_MoveSpeed = 1.0f;
	float m_LegSwing = 0.0f;
	float m_LastLegSwing = 0.0f;

	int m_PathExpireTicks = 0;

	float m_InterpDeadTicks = 0;
	int m_DeadTicks = 0;
	int m_LastDeadTicks = 0;
	int m_InactivityTicks = 0;
	int m_TauntTimer = 0;
	bool m_FollowPath = true;
	bool m_BurnsInDaylight = true;

	float m_LastSentLegSwing = 0.0f;
	int m_LastSentDeadTicks = 0;

	NavPath m_CurrentPath;
END_ENTITY_DECLARATION