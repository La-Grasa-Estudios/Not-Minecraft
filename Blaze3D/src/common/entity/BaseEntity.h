#pragma once

#include "AABB.h"
#include "common/ItemStack.h"
#include "common/path/NavPath.h"
#include <common/nbt/TagCompound.h>

#include <functional>
#include <unordered_map>
#include <string>

#include "thirdparty/glm/ext.hpp"

class World;
class riDevice;

enum class DamageSource
{
	GENERIC,
	FALL,
	DROWN,
	MOB,
	FIRE,
	EXPLOSION,
};

enum class InteractionType
{
	NONE,
	CRAFTING,
	CHEST,
	FURNACE
};

#define DECLARE_ENTITY_CONSTRUCTOR(type, fn) \
namespace Internal { \
    struct internalEntityConstructorFor##type { \
        internalEntityConstructorFor##type() { \
             BaseEntity::RegisterEntityConstructor(#type, fn); \
        } \
    }; \
    static internalEntityConstructorFor##type objReg##type; \
	static volatile void* dummy##type = &objReg##type; \
}

#define BEGIN_ENTITY_DECLARATION(name) \
class  name##Renderer;\
class name : public BaseEntity \
{	\
friend  name##Renderer;

#define BEGIN_ENTITY_DECLARATION_WITH_CLASS(name, clazz) \
class  name##Renderer;\
class name : public clazz \
{	\
friend  name##Renderer;

#define END_ENTITY_DECLARATION };

class BaseEntity;

class EntityInventory
{
	int AddStackInternal(ItemStack& stack, int depth);

public:
	ItemStack Resources[9];
	ItemStack InventoryResources[27];

	ItemStack Armor[4];

	bool AddStack(ItemStack& stack);
	void DropAllItems(BaseEntity* entity, World* world);

	bool ConsumeItemWithId(int id);

	int GetArmorValue();
	void DamageArmor(int amount);
};

class BaseEntity
{
public:
	BaseEntity();

	virtual void Update();
	virtual void OnInteractWithPlayer(BaseEntity* other);
	float GetLightLevel();

	glm::vec3 LastPosition = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 LastRotation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
	glm::vec3 LastVelocity = {};
	glm::vec3 Velocity = {};

	AABB BoundingBox;

	EntityInventory Inventory;
	World* EntityWorld = NULL;

	float BoundingBoxWidth;
	float BoundingBoxHeight;

	float HeightOffset = 1.62f;
	float StepHeight = 0.0f;

	float WalkDist = 0.0f;
	float LastWalkDist = 0.0f;
	float StepDist = 0.0f;
	int StepCount = 0;
	int TicksAlive = 0;
	int TicksDead = 0;

	int Health = 20;
	int ImmunityTicks = 0;
	int DamageTicks = 0;
	int LastDamageTicks = 0;
	int RemainingFireTicks = 0;
	int Dimension = 0;
	float FallDistance = 0.0f;

	bool HorizontalCollision = false;
	bool IsGrounded = false;
	bool IsInWater = false;
	bool IsInLava = false;
	bool IsInLiquid = false;
	bool IsImportant = false; // If true, entity is able to bypass entity limit
	bool IsAbleToMakeStepSound = true;
	bool IsNoclipEnabled = false;
	bool IsDead = false;
	bool IsRemovable = true;
	bool CanBeRaycasted = true;
	bool CanBeHit = true;
	bool ForceRemove = false;
	bool AllowSliding = true;

	void SetPosition(glm::vec3 dest);
	void MoveRelative(float xa, float za, float speed);
	void PushEntity(BaseEntity* other, float force);
	int GetEntityFacingDirection();

	template<typename T>
	T Interpolate(T a, T b, float t)
	{
		return a + (b - a) * t;
	}

	glm::vec3 GetForward();

	virtual void WriteToNbt(TagCompound& tag);
	virtual void ReadFromNbt(TagCompound& tag);
	virtual bool IsSerializable() { return true; };

	static std::shared_ptr<BaseEntity> ReadEntity(TagCompound& tag, World* world);
	static void RegisterEntityConstructor(const char* type, std::function<std::shared_ptr<BaseEntity>(World*, TagCompound&)> fn);
	static std::shared_ptr<BaseEntity> CreateEntityByType(const std::string& type, World* world);

	virtual bool TakeDamage(BaseEntity* other, int amount, DamageSource source, bool skipImmunity = false);
	virtual void Attack(BaseEntity* other, int amount);
	virtual bool AddHealth(int amount);

	virtual void OnDeath();
	virtual void OnPathReady(NavPath& path);
	virtual void OnPush(BaseEntity* other, float x, float z);

	virtual void InteractWith(InteractionType type, int x, int y, int z) {};

	uint64_t GetUUID() const { return m_UUID; }
	void SetUUID(uint64_t uuid) { m_UUID = uuid; } // Only used to spawn network entities

	bool IsBurning();

	size_t GetEntityHashCode();

protected:

	void RenderBurningFire(riDevice* device);

	float m_IterpDamageTicks = 0.0f;

	int m_DamageTaken = 0;

	uint64_t m_UUID = 0;

	int m_ChannelStepAudio = -1;

	void Move();

	void UpdatePositionBasedOnAABB();
	void UpdateAABBBasedOnBBParams();
	glm::vec3 FindClosestPassableSpace(int searchDistance, bool searchY);

private:

	static inline std::unordered_map<std::string, std::function<std::shared_ptr<BaseEntity>(World*, TagCompound&)>> s_EntityIDToFnMap;

};