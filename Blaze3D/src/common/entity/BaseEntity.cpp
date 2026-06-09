#include "BaseEntity.h"
#include "common/World.h"
#include "common/nbt/NBT.h"
#include "common/item/Item.h"

#include "ItemEntity.h"

#include "engine/Time.h"
#include "common/Random.h"

#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
#endif

int EntityInventory::AddStackInternal(ItemStack& toAdd, int depth)
{
	if (depth > 64)
	{
		return depth;
	}
	for (int i = 0; i < 27 && toAdd.Amount > 0; i++)
	{
		auto& stack = InventoryResources[i];
		if (stack.Id == toAdd.Id && stack.HasSpace(toAdd.Amount))
		{
			toAdd.Amount = stack.Expand(toAdd.Amount);
			break;
		}
	}
	for (int i = 0; i < 9 && toAdd.Amount > 0; i++)
	{
		auto& stack = Resources[i];
		if (stack.Id == toAdd.Id && stack.HasSpace(toAdd.Amount))
		{
			toAdd.Amount = stack.Expand(toAdd.Amount);
			break;
		}
	}
	for (int i = 0; i < 9 && toAdd.Amount > 0; i++)
	{
		auto& stack = Resources[i];
		if (stack.Id == 0 && stack.HasSpace(toAdd.Amount))
		{
			stack.Id = toAdd.Id;
			stack.Damage = toAdd.Damage;
			toAdd.Amount = stack.Expand(toAdd.Amount);
			break;
		}
	}
	for (int i = 0; i < 27 && toAdd.Amount > 0; i++)
	{
		auto& stack = InventoryResources[i];
		if (stack.Id == 0 && stack.HasSpace(toAdd.Amount))
		{
			stack.Id = toAdd.Id;
			stack.Damage = toAdd.Damage;
			toAdd.Amount = stack.Expand(toAdd.Amount);
			break;
		}
	}
	if (toAdd.Amount > 0)
	{
		return this->AddStackInternal(toAdd, depth + 1);
	}
	return depth;
}
bool EntityInventory::AddStack(ItemStack& toAdd)
{
	return AddStackInternal(toAdd, 0) <= 64;
}

void EntityInventory::DropAllItems(BaseEntity* entity, World* world)
{
	for (int i = 0; i < 27; i++)
	{
		auto& stack = InventoryResources[i];
		if (stack.Id == 0) continue;
		auto item = std::make_shared<ItemEntity>(world, entity->Position, stack);
		stack = {};
		stack.IsDirty = true;
		world->AddEntity(item);
	}
	for (int i = 0; i < 9; i++)
	{
		auto& stack = Resources[i];
		if (stack.Id == 0) continue;
		auto item = std::make_shared<ItemEntity>(world, entity->Position, stack);
		stack = {};
		stack.IsDirty = true;
		world->AddEntity(item);
	}
	for (int i = 0; i < 4; i++)
	{
		auto& stack = Armor[i];
		if (stack.Id == 0) continue;
		auto item = std::make_shared<ItemEntity>(world, entity->Position, stack);
		stack = {};
		stack.IsDirty = true;
		world->AddEntity(item);
	}
}

bool EntityInventory::ConsumeItemWithId(int id)
{
	for (int i = 0; i < 27; i++)
	{
		auto& stack = InventoryResources[i];
		if (stack.Id != id) continue;
		stack.Shrink(1);
		return true;
	}
	for (int i = 0; i < 9; i++)
	{
		auto& stack = Resources[i];
		if (stack.Id != id) continue;
		stack.Shrink(1);
		return true;
	}
	return false;
}

int EntityInventory::GetArmorValue()
{
	int totalArmor = 0;
	int totalArmorDurabilityLeft = 0;
	int totalArmorMaxDamage = 0;

	for (int i = 0; i < 4; i++)
	{
		auto& stack = Armor[i];
		if (stack.Id == 0) continue;
		auto item = Item::GetItem(stack.Id);
		if (item && item->GetType() >= TOOL_TYPE_ARMOR_HEAD && item->GetType() <= TOOL_TYPE_ARMOR_FEET)
		{
			auto armor = static_cast<ItemArmor*>(item);
			totalArmor += armor->ProtectionLevel;

			int armorDurabilityLeft = armor->GetMaxDamage() - stack.Damage;

			totalArmorDurabilityLeft += armorDurabilityLeft;
			totalArmorMaxDamage += armor->GetMaxDamage();
		}
	}

	if (totalArmorMaxDamage == 0)
	{
		return 0;
	}
	else
	{
		return (totalArmor - 1) * totalArmorDurabilityLeft / totalArmorMaxDamage + 1;
	}
}

void EntityInventory::DamageArmor(int amount)
{
	for (int i = 0; i < 4; i++)
	{
		auto& stack = Armor[i];
		for (int j = 0; j < amount; j++)
		{
			stack.IncreaseDamage();
		}
	}
}

BaseEntity::BaseEntity() {
	BoundingBoxWidth = 0.6f;
	BoundingBoxHeight = 1.6f;
	BoundingBox = {};
	Inventory = {};
	static Random rand;
	m_UUID = rand.NextLong() + rand.NextLong() + rand.NextLong();
}

void BaseEntity::Update() {

}

void BaseEntity::OnInteractWithPlayer(BaseEntity*) {

}

float BaseEntity::GetLightLevel()
{
	int light = EntityWorld->GetMixedLight((int)floor(Position.x), (int)Position.y, (int)floor(Position.z));
	return EntityWorld->LightLUT[light];
}

void BaseEntity::SetPosition(glm::vec3 dest)
{
	Position = dest;
	UpdateAABBBasedOnBBParams();
	LastPosition = Position;
}

void BaseEntity::MoveRelative(float xa, float za, float speed)
{
	float dist = xa * xa + za * za;
	if (dist < 0.01F) {
		return;
	}
	dist = speed / sqrt(dist);
	xa *= dist;
	za *= dist;

	float sin0 = sin(Rotation.y * 3.141592653589793f / 180.0f);
	float cos0 = cos(Rotation.y * 3.141592653589793f / 180.0f);

	Velocity.x += xa * cos0 - za * sin0;
	Velocity.z += za * cos0 + xa * sin0;
}

void BaseEntity::PushEntity(BaseEntity* other, float force)
{
	glm::vec2 delta = { other->Position.x - Position.x, other->Position.z - Position.z };
	float dist = glm::length(delta);
	if (dist >= 0.01f)
	{
		delta /= dist;
		float pushFactor = (1.0f / dist);
		if (pushFactor > 1.0f)
			pushFactor = 1.0f;
		pushFactor *= 0.05f * force;
		other->Velocity.x += delta.x * pushFactor;
		other->Velocity.z += delta.y * pushFactor;
		other->OnPush(this, delta.x * pushFactor, delta.y * pushFactor);
	}
}

int BaseEntity::GetEntityFacingDirection()
{
	auto forward = GetForward();
	auto angle = glm::atan(forward.x, -forward.z) * 180.0f / glm::pi<float>();
	if (angle < 0.0f)
		angle += 360.0f;

	int facing = 0; // North

	if (angle > 135.0f && angle < 225.0f)
	{
		facing = 1; // South
	}
	if (angle > 225.0f && angle < 315.0f)
	{
		facing = 2; // East
	}
	if (angle > 45.0f && angle < 135.0f)
	{
		facing = 3; // West
	}

	return facing;
}

glm::vec3 BaseEntity::GetForward()
{

	const float Deg2Rad = (float)3.141592653589793 * 2.0f / 360.0f;

	float x = -sin(-Rotation.y * Deg2Rad) * cos(Rotation.x * Deg2Rad);
	float y = sin(-Rotation.x * Deg2Rad);
	float  z = -cos(Rotation.x * Deg2Rad) * cos(-Rotation.y * Deg2Rad);

	return { x, y, z };

}

void BaseEntity::WriteToNbt(TagCompound& tag)
{
	auto position = std::make_shared<TagList>("Pos", NBT_TAG_TYPE_DOUBLE);
	auto rotation = std::make_shared<TagList>("Rotation", NBT_TAG_TYPE_FLOAT);
	auto motion = std::make_shared<TagList>("Motion", NBT_TAG_TYPE_DOUBLE);

	position->Add(std::make_shared<TagDouble>("", Position.x));
	position->Add(std::make_shared<TagDouble>("", Position.y));
	position->Add(std::make_shared<TagDouble>("", Position.z));

	rotation->Add(std::make_shared<TagFloat>("", Rotation.x));
	rotation->Add(std::make_shared<TagFloat>("", Rotation.y));

	motion->Add(std::make_shared<TagDouble>("", Velocity.x));
	motion->Add(std::make_shared<TagDouble>("", Velocity.y));
	motion->Add(std::make_shared<TagDouble>("", Velocity.z));

#if defined(__GNUC__) || defined(__clang__)
	int status;
	char* realname = abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
	std::string result(realname ? realname : typeid(*this).name());
	free(realname);
	tag.SetTag(std::make_shared<TagString>("EntityID", result));
#pragma message "Using CLANG|GCC demangling"
#else
	std::string className = typeid(*this).name();
	className = className.substr(className.find_first_of(' ') + 1);
	tag.SetTag(std::make_shared<TagString>("EntityID", className));
#endif

	tag.SetInt("TicksAlive", TicksAlive);
	tag.SetInt("Health", Health);
	tag.SetInt("ImmunityTicks", ImmunityTicks);
	tag.SetInt("DamageTicks", DamageTicks);
	tag.SetInt("Dimension", Dimension);
	tag.SetShort("RemainingBurnTime", RemainingFireTicks);
	tag.SetLong("UUID", m_UUID);
	tag.SetTag(position);
	tag.SetTag(rotation);
	tag.SetTag(motion);
	tag.SetTag(std::make_shared<TagByte>("OnGround", IsGrounded));

	auto inventory = std::make_shared<TagList>("Inventory", NBT_TAG_TYPE_COMPOUND);

	for (size_t i = 0; i < 9; i++)
	{
		inventory->Add(Inventory.Resources[i].WriteToTag());
	}
	for (size_t i = 0; i < 27; i++)
	{
		inventory->Add(Inventory.InventoryResources[i].WriteToTag());
	}
	for (size_t i = 0; i < 4; i++)
	{
		inventory->Add(Inventory.Armor[i].WriteToTag());
	}

	tag.SetTag(inventory);
}

void BaseEntity::ReadFromNbt(TagCompound& tag)
{
	auto& position = tag.GetTagAs<TagList>("Pos");
	auto& rotation = tag.GetTagAs<TagList>("Rotation");
	auto& motion = tag.GetTagAs<TagList>("Motion");

	Health = tag.GetInt("Health");
	ImmunityTicks = tag.GetInt("ImmunityTicks");
	DamageTicks = tag.GetInt("DamageTicks");
	auto lastUUID = m_UUID;
	m_UUID = tag.GetLong("UUID");
	if (m_UUID == 0)
	{
		m_UUID = lastUUID;
	}

	Position[0] = static_cast<float>(position.GetAs<TagDouble>(0));
	Position[1] = static_cast<float>(position.GetAs<TagDouble>(1));
	Position[2] = static_cast<float>(position.GetAs<TagDouble>(2));

	Rotation[0] = rotation.GetAs<TagFloat>(0);
	Rotation[1] = rotation.GetAs<TagFloat>(1);

	Velocity[0] = static_cast<float>(motion.GetAs<TagDouble>(0));
	Velocity[1] = static_cast<float>(motion.GetAs<TagDouble>(1));
	Velocity[2] = static_cast<float>(motion.GetAs<TagDouble>(2));

	IsGrounded = static_cast<bool>(tag.GetTagAs<TagByte>("OnGround"));
	TicksAlive = tag.GetInt("TicksAlive");
	Dimension = tag.GetInt("Dimension");
	RemainingFireTicks = tag.GetShort("RemainingBurnTime");

	auto& inventory = tag.GetTagAs<TagList>("Inventory");

	for (size_t i = 0; i < 9; i++)
	{
		Inventory.Resources[i].ReadFromTag(inventory.GetAs<TagCompound>(i));
	}
	for (size_t i = 0; i < 27; i++)
	{
		Inventory.InventoryResources[i].ReadFromTag(inventory.GetAs<TagCompound>(i + 9));
	}
	if (inventory.Size() > 9 + 27)
	{
		for (size_t i = 0; i < 4; i++)
		{
			Inventory.Armor[i].ReadFromTag(inventory.GetAs<TagCompound>(i + 9 + 27));
		}
	}

	SetPosition(Position);
	LastRotation = Rotation;
}

std::shared_ptr<BaseEntity> BaseEntity::ReadEntity(TagCompound& tag, World* world)
{
	if (!tag.Contains("EntityID"))
		return NULL;

	auto id = std::string(tag.GetTagAs<TagString>("EntityID"));

	if (auto fn = s_EntityIDToFnMap.find(id); fn != s_EntityIDToFnMap.end())
	{
		return fn->second(world, tag);
	}
	else
	{
		printf("Warning: Could not find entity constructor for ID '%s' this is a bug!\n", id.c_str());
	}

	return NULL;
}

void BaseEntity::RegisterEntityConstructor(const char* type, std::function<std::shared_ptr<BaseEntity>(World*, TagCompound&)> fn)
{
	s_EntityIDToFnMap[type] = fn;
}

std::shared_ptr<BaseEntity> BaseEntity::CreateEntityByType(const std::string& type, World* world)
{
	if (auto fn = s_EntityIDToFnMap.find(type); fn != s_EntityIDToFnMap.end())
	{
		TagCompound tag;
		return fn->second(world, tag);
	}
	else
	{
		printf("Warning: Could not find entity constructor for ID '%s' this is a bug!\n", type.c_str());
	}	

	return NULL;
}

bool BaseEntity::TakeDamage(BaseEntity* other, int amount, DamageSource source, bool skipImmunity)
{
	if (ImmunityTicks <= 0)
	{
		m_DamageTaken = 0;
	}

	int orig = amount;

	bool doImmunity = ImmunityTicks > 0 && m_DamageTaken >= amount;
	if (amount > m_DamageTaken && m_DamageTaken > 0)
	{
		amount -= m_DamageTaken;
		printf("Damage is higher than before, ignoring immunity and inflicting damage %i\n", amount);
	}
	if (amount <= 0 || (doImmunity > 0 && !skipImmunity) || Health <= 0)
	{
		return false;
	}
	Health -= amount;
	if (Health < 0)
	{
		Health = 0;
	}

	DamageTicks = 10;

	if (!skipImmunity && ImmunityTicks == 0)
		ImmunityTicks = 10;
	
	if (m_DamageTaken < orig)
		m_DamageTaken = orig;

	return true;
}

void BaseEntity::Attack(BaseEntity* other, int amount)
{
}

bool BaseEntity::AddHealth(int amount)
{
	if (Health >= 20)
		return false;
	Health += amount;
	Health = glm::min(Health, 20);
	return true;
}

void BaseEntity::OnDeath()
{

}

void BaseEntity::OnPathReady(NavPath& path)
{
}

void BaseEntity::OnPush(BaseEntity* other, float x, float z)
{
}

bool BaseEntity::IsBurning()
{
	return RemainingFireTicks > 0 || IsInLava;
}

size_t BaseEntity::GetEntityHashCode()
{
	return typeid(this).hash_code();
}

// Helper to wrap a single angle delta to [-180, 180]
float ShortestAngleDist(float target, float source) {
	float delta = glm::mod(target - source + 180.f, 360.f);
	if (delta < 0) delta += 360.f;
	return delta - 180.f;
}

// Helper to interpolate a vec3 of Euler angles
glm::vec3 InterpolateEuler(glm::vec3 a, glm::vec3 b, float alpha) {
	glm::vec3 result;
	result.x = a.x + ShortestAngleDist(b.x, a.x) * alpha;
	result.y = a.y + ShortestAngleDist(b.y, a.y) * alpha;
	result.z = a.z + ShortestAngleDist(b.z, a.z) * alpha;
	return result;
}

void BaseEntity::RenderBurningFire(riDevice* device)
{
	if (!IsBurning())
	{
		return;
	}	
}

void BaseEntity::Move() {
	if (!EntityWorld) return;

	TicksAlive++;

	if (IsNoclipEnabled)
	{
		BoundingBox.move(Velocity.x, Velocity.y, Velocity.z);
		UpdatePositionBasedOnAABB();
		IsGrounded = false;
		return;
	}

	AABB bbs[64];
	int count = 64;
	EntityWorld->GetBoxes(BoundingBox.expand(Velocity.x, Velocity.y, Velocity.z), bbs, count);

	bool wasDead = IsDead;
	IsDead = Health <= 0;

	if (!wasDead && IsDead)
	{
		OnDeath();
	}

	if (IsDead)
	{
		TicksDead++;
	}
	else
	{
		TicksDead = 0;
	}

	float xa = Velocity.x;
	float ya = Velocity.y;
	float za = Velocity.z;

	float xa1 = xa;
	float ya1 = ya;
	float za1 = za;

	for (int i = 0; i < count; i++) {
		ya = bbs[i].clipYCollide(BoundingBox, ya);
	}

	BoundingBox.move(0, ya, 0);
	if (ya != ya1 && !AllowSliding)
	{
		xa = ya = za = 0;
	}

	for (int i = 0; i < count; i++) {
		xa = bbs[i].clipXCollide(BoundingBox, xa);
	}

	BoundingBox.move(xa, 0, 0);

	if (xa != xa1 && !AllowSliding)
	{
		xa = ya = za = 0;
	}

	for (int i = 0; i < count; i++) {
		za = bbs[i].clipZCollide(BoundingBox, za);
	}

	BoundingBox.move(0, 0, za);

	if (za != za1 && !AllowSliding)
	{
		xa = ya = za = 0;
	}

	bool wasGrounded = IsGrounded;
	IsGrounded = ya1 != ya && ya1 < 0.0;
	HorizontalCollision = (xa1 != xa || za1 != za);

	if (StepHeight > 0.0f && IsGrounded && HorizontalCollision && (xa1 != xa || za1 != za))
	{
		float bx = xa;
		float by = ya;
		float bz = za;
		xa = xa1;
		ya = StepHeight;
		za = za1;

		count = 64;
		EntityWorld->GetBoxes(BoundingBox.expand(xa, ya, za), bbs, count);

		auto copy = BoundingBox;

		for (int i = 0; i < count; i++) {
			ya = bbs[i].clipYCollide(copy, ya);
		}

		copy.move(0, ya, 0);

		if (ya != by && !AllowSliding)
		{
			xa = ya = za = 0;
		}

		for (int i = 0; i < count; i++) {
			xa = bbs[i].clipXCollide(copy, xa);
		}

		copy.move(xa, 0, 0);

		if (xa != bx && !AllowSliding)
		{
			xa = ya = za = 0;
		}

		for (int i = 0; i < count; i++) {
			za = bbs[i].clipZCollide(copy, za);
		}

		copy.move(0, 0, za);

		if (za != bz && !AllowSliding)
		{
			xa = ya = za = 0;
		}

		// Before velocity was higher, that means we collided with something when trying to go up a step
		// Revert
		if (bx * bx + bz * bz >= xa * xa + za * za)
		{
			xa = bx;
			ya = by;
			za = bz;
		}
		else // Snap to block
		{
			BoundingBox = copy;
			ya = -StepHeight;
			for (int i = 0; i < count; i++) {
				ya = bbs[i].clipYCollide(BoundingBox, ya);
			}
			BoundingBox.move(0, ya, 0);
		}
	}

	if (!IsGrounded && ya < 0.0f)
	{
		FallDistance -= ya;
	}

	LastVelocity = Velocity;
	Velocity.x = (float)xa;
	Velocity.y = (float)ya;
	Velocity.z = (float)za;

	if (Velocity.x != xa1)
	{
		Velocity.x = 0.0f;
	}

	if (Velocity.y != ya1)
	{
		Velocity.y = 0.0f;
	}

	if (Velocity.z != za1)
	{
		Velocity.z = 0.0f;
	}

	auto liquidType = EntityWorld->IsBoundingBoxInLiquid(BoundingBox);

	bool wasInWater = IsInWater;

	IsInWater = liquidType == LIQUID_TYPE_WATER;
	IsInLava = liquidType == LIQUID_TYPE_LAVA;
	IsInLiquid = IsInWater || IsInLava;

	if (IsInWater)
	{
		FallDistance = 0.0f;
		RemainingFireTicks = 0;
	}

	if (!wasInWater && IsInWater)
	{
		EntityWorld->PlaySound("misc.splash", Position, 0.1f);
	}
	if (!IsInWater && !wasGrounded && IsGrounded)
	{
		int damage = (int)floor(FallDistance - 2.0f);
		if (damage > 0)
		{
			TakeDamage(NULL, damage, DamageSource::FALL, true);
		}
	}
	if (IsGrounded)
	{
		FallDistance = 0;
	}

	LastPosition = Position;
	UpdatePositionBasedOnAABB();

	LastWalkDist = WalkDist;
	WalkDist += glm::distance(glm::vec2(LastPosition.x, LastPosition.z), glm::vec2(Position.x, Position.z));

	Block* below = Block::GetBlock(EntityWorld->GetBlock(glm::floor(Position.x), glm::floor(BoundingBox.minY - 0.2f), glm::floor(Position.z)).Id);

	if (WalkDist * 0.6f > StepCount)
	{
		Block* at = Block::GetBlock(EntityWorld->GetBlock(glm::floor(Position.x), glm::floor(BoundingBox.minY + 0.2f), glm::floor(Position.z)).Id);
		StepCount++;
		if (IsAbleToMakeStepSound && at && at->SoundDef && at->MakesSound())
		{
			EntityWorld->PlaySound(at->SoundDef->StepSound, Position, 0.1f);
		} else
		if (IsAbleToMakeStepSound && below && below->SoundDef && below->MakesSound())
		{
			EntityWorld->PlaySound(below->SoundDef->StepSound, Position, 0.1f);
		}
	}

	if (below && below->GetId() == BLOCK_SOUL_SAND)
	{
		Velocity.x *= 0.4f;
		Velocity.z *= 0.4f;
	}

	LastDamageTicks = DamageTicks;
	if (DamageTicks > 0)
	{
		DamageTicks--;
	}

	if (ImmunityTicks > 0)
	{
		ImmunityTicks--;
	}
}

void BaseEntity::UpdatePositionBasedOnAABB() {
	AABB& bb = BoundingBox;
	glm::vec3& pos = Position;

	pos.x = (float)(bb.minX + bb.maxX) / 2.0f;
	pos.y = (float)bb.minY + HeightOffset;
	pos.z = (float)(bb.minZ + bb.maxZ) / 2.0f;
}

void BaseEntity::UpdateAABBBasedOnBBParams()
{
	const double w = BoundingBoxWidth / 2.0;
	const double h = BoundingBoxHeight / 2.0;
	Position.y = Position.y - HeightOffset + h;
	BoundingBox = AABB(Position.x - w, Position.y - h, Position.z - w, Position.x + w, Position.y + h, Position.z + w);
	UpdatePositionBasedOnAABB();
}

glm::vec3 BaseEntity::FindClosestPassableSpace(int searchDistance, bool searchY)
{
	auto pos = glm::ivec3(glm::floor(Position));

	int iter = 1;

	while (iter <= searchDistance)
	{
		if (EntityWorld->GetBlock(pos.x - iter, pos.y, pos.z).Id == 0)
		{
			return glm::vec3(pos.x - iter, pos.y, pos.z) + glm::vec3(0.5f);
		}
		if (EntityWorld->GetBlock(pos.x + iter, pos.y, pos.z).Id == 0)
		{
			return glm::vec3(pos.x + iter, pos.y, pos.z) + glm::vec3(0.5f);
		}
		if (EntityWorld->GetBlock(pos.x, pos.y, pos.z - iter).Id == 0)
		{
			return glm::vec3(pos.x, pos.y, pos.z - iter) + glm::vec3(0.5f);
		}
		if (EntityWorld->GetBlock(pos.x, pos.y, pos.z + iter).Id == 0)
		{
			return glm::vec3(pos.x, pos.y, pos.z + iter) + glm::vec3(0.5f);
		}
		if (searchY)
		{
			if (EntityWorld->GetBlock(pos.x, pos.y + iter, pos.z).Id == 0)
			{
				return glm::vec3(pos.x, pos.y + iter, pos.z) + glm::vec3(0.5f);
			}
		}
		iter++;
	}

	return glm::vec3(pos) + glm::vec3(0.5f);
}

