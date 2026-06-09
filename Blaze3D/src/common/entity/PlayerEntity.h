#pragma once

#include "BaseEntity.h"
#include "engine/System.h"

class TagCompound;

class PlayerEntity : public BaseEntity {

public:

	PlayerEntity(World* world);

	sysInputData inputState;

	void Update() override;

	float LastBob = 0.0f;
	float Bob = 0.0f;
	int HotbarSlot = 0;
	int SendHotbarSlot = 0;

	int NetherPortalTicks = 0;
	int LastNetherPortalTicks = 0;
	int FireTicks = -20;

	float SwingFactor = 0.0f;
	float LastSwingFactor = 0.0f;

	bool IsOnline = false;

	ItemStack CursorStack;

	void WriteToNbt(TagCompound& tag) override;
	void ReadFromNbt(TagCompound& tag) override;
	bool IsSerializable() override;
	bool TakeDamage(BaseEntity* other, int amount, DamageSource source, bool skipImmunity = false) override;
	float GetCameraTilt(float a);
	void OnDeath() override;
	void Attack(BaseEntity* other, int amount) override;
	void DropItemStack(ItemStack& stack);
	void InteractWith(InteractionType type, int x, int y, int z);

	int GetBlockAtEyePos();
};