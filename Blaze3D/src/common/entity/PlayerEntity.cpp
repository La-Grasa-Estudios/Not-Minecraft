#include "PlayerEntity.h"
#include "common/World.h"

#include "client/Client.h"
#include "client/ui/ScreenGameOver.h"
#include "client/ui/ScreenCrafting.h"
#include "client/ui/ScreenFurnace.h"
#include "client/ui/ScreenContainer.h"
#include "common/nbt/NBT.h"
#include "common/item/Item.h"
#include "common/entity/ItemEntity.h"

PlayerEntity::PlayerEntity(World* world) : BaseEntity() {

	EntityWorld = world;
	StepHeight = 0.5f;
	SetPosition({ 64, 64, 64 });
	IsImportant = true;
	IsRemovable = false;
}

void PlayerEntity::Update()
{
	if (EntityWorld->GetChunk((int)floor(Position.x), (int)floor(Position.z)) == NULL
		|| EntityWorld->GetChunk((int)floor(Position.x), (int)floor(Position.z))->GenStage != TERRAIN_GEN_COMPLETE)
		return;

	if (IsOnline)
	{
		if (!IsDead)
		{
			int count = 0;
			auto entities = EntityWorld->GetCollidingEntitiesWith(BoundingBox.grow(1.0f, 0.0f, 1.0f), &count);

			for (int i = 0; i < count; i++)
			{
				entities[i]->OnInteractWithPlayer(this);
			}
		}

		UpdateAABBBasedOnBBParams();

		return;
	}

	if (EntityWorld->IsOutsideWorld((int)floor(Position.x), (int)floor(Position.z)))
	{
		SetPosition(glm::vec3(EntityWorld->SpawnX, EntityWorld->SpawnY, EntityWorld->SpawnZ));
	}

	float xa = inputState.MovementAxisX;
	float za = inputState.MovementAxisY;

	if (IsDead)
	{
		xa = 0.0f;
		za = 0.0f;
	}
	else
	{
		if (inputState.MovementAxisJump) {
			if (IsGrounded && !IsInLiquid) {
				Velocity.y = 0.42f;
			}
			if (IsInLiquid)
			{
				Velocity.y += 0.04f;
			}
		}
	}

	LastSwingFactor = SwingFactor;
	if (glm::length(glm::vec2(xa, za)) > 0.01f && IsGrounded && BoundingBoxHeight > 1.6f)
	{
		SwingFactor += 0.4f;
	}
	else
	{
		SwingFactor -= 0.4f;
	}
	SwingFactor = glm::clamp(SwingFactor, 0.0f, 1.0f);

	float mul = 1.0f;

	if (IsGrounded) {
		mul = 0.8f;
	}
	else {
		mul = 0.9f;
	}

	float speed = 0.1f;

	if (IsInLiquid)
	{
		speed = 0.02f;
	}

	if (!IsInLiquid && !IsGrounded)
	{
		speed = 0.02f;
	}

	int lastHeight = BoundingBoxHeight * 10;

	auto bb1 = BoundingBox;
	if (inputState.ShiftAction || BoundingBoxHeight < 1.6f)
	{
		bb1.maxY += 0.3f;
	}

	bool forceCrouch = !EntityWorld->IsBoundingBoxFree(bb1);
	bool crouching = forceCrouch || inputState.ShiftAction;

	if (inputState.ShiftAction || forceCrouch)
	{
		crouching = true;
		BoundingBoxHeight = 1.5f;
		IsAbleToMakeStepSound = false;
	}
	else if (!forceCrouch)
	{
		BoundingBoxHeight = 1.8f;
		IsAbleToMakeStepSound = true;
	}

	if (crouching)
	{
		speed *= 0.33f;
	}

	if (BoundingBoxHeight * 10 != lastHeight)
	{
		UpdateAABBBasedOnBBParams();

		if (inputState.ShiftAction || forceCrouch)
		{
			HeightOffset = 1.4f;
		}
		else
		{
			HeightOffset = 1.62f;
		}
	}

	if (IsInWater)
	{
		MoveRelative(xa, za, speed);
		Move();
		Velocity.y *= 0.8f;
		Velocity.x *= 0.8f;
		Velocity.z *= 0.8f;
		Velocity.y -= 0.02f;
		if (!IsInLiquid && HorizontalCollision)
		{
			Velocity.y = 0.3f;
		}
		FallDistance = 0.0f;
	}
	else if (IsInLava)
	{
		MoveRelative(xa, za, speed);
		Move();
		Velocity *= 0.5f;
		Velocity.y -= 0.02f;
		if (!IsInLiquid && HorizontalCollision)
		{
			Velocity.y = 0.3f;
		}
	}
	else
	{
		MoveRelative(xa, za, speed);

		if (IsGrounded && inputState.ShiftAction)
		{
			auto bb = BoundingBox;
			bb.move(glm::sign(Velocity.x) * 0.1f, -0.625f, 0.0f);

			if (EntityWorld->IsBoundingBoxFree(bb))
			{
				Velocity.x = 0.0f;
			}

			bb = BoundingBox;
			bb.move(0.0f, -0.625f, glm::sign(Velocity.z) * 0.1f);

			if (EntityWorld->IsBoundingBoxFree(bb))
			{
				Velocity.z = 0.0f;
			}
		}

		Move();
		Velocity.y *= 0.98f;
		Velocity.x *= 0.91f;
		Velocity.z *= 0.91f;
		Velocity.y -= 0.08f;
		if (IsGrounded)
		{
			Velocity.x *= 0.6f;
			Velocity.z *= 0.6f;
		}
	}

	if (!IsDead)
	{
		int count = 0;
		auto entities = EntityWorld->GetCollidingEntitiesWith(BoundingBox.grow(1.0f, 0.0f, 1.0f), &count);

		for (int i = 0; i < count; i++)
		{
			entities[i]->OnInteractWithPlayer(this);
		}
	}

	if (IsInLava)
	{
		if (EntityWorld->IsServerWorld)
			TakeDamage(NULL, 4, DamageSource::GENERIC);
		RemainingFireTicks = 300;
	}

	if (RemainingFireTicks > 0)
		RemainingFireTicks--;

	if (IsBurning() && RemainingFireTicks % 20 == 0)
	{
		if (EntityWorld->IsServerWorld)
			TakeDamage(NULL, 1, DamageSource::GENERIC);
	}

	auto pos = glm::ivec3(glm::floor(Position));

	LastNetherPortalTicks = NetherPortalTicks;
	if (EntityWorld->GetBlock(pos.x, pos.y, pos.z).Id == BLOCK_PORTAL ||
		EntityWorld->GetBlock(pos.x, pos.y - 1, pos.z).Id == BLOCK_PORTAL)
	{
		if (NetherPortalTicks == 0)
		{
			EntityWorld->PlaySoundEffect("misc.portal.trigger", 0.1f);
		}
		NetherPortalTicks++;
	}
	else
	{
		NetherPortalTicks -= 2;
	}

	if (EntityWorld->GetBlock(pos.x, pos.y, pos.z).Id == BLOCK_FIRE ||
		EntityWorld->GetBlock(pos.x, pos.y - 1, pos.z).Id == BLOCK_FIRE)
	{
		FireTicks++;
	}
	else
	{
		FireTicks = -5;
	}

	if (FireTicks > 0)
	{
		RemainingFireTicks = 160;
		if (EntityWorld->IsServerWorld)
			TakeDamage(NULL, 1, DamageSource::FIRE);
	}

	NetherPortalTicks = glm::clamp(NetherPortalTicks, 0, 99);

	if (!EntityWorld->IsBoundingBoxFree(BoundingBox))
	{
		auto point = FindClosestPassableSpace(1, false);
		Velocity.x += (point.x - Position.x) * 0.05f;
		Velocity.z += (point.z - Position.z) * 0.05f;
	}

	if (Block::GetBlock(GetBlockAtEyePos())->IsFullyOpaqueBlock())
	{
		if (EntityWorld->IsServerWorld)
			TakeDamage(NULL, 1, DamageSource::FALL);
	}
}

void PlayerEntity::WriteToNbt(TagCompound& tag)
{
	auto posY = Position.y;
	Position.y = Position.y - HeightOffset + 1.8f;
	BaseEntity::WriteToNbt(tag);
	tag.SetTag(std::make_shared<TagByte>("Slot", HotbarSlot));
	Position.y = posY;
}

void PlayerEntity::ReadFromNbt(TagCompound& tag)
{
	BaseEntity::ReadFromNbt(tag);
	HotbarSlot = tag.GetTagAs<TagByte>("Slot");
}

bool PlayerEntity::IsSerializable()
{
	return false;
}

bool PlayerEntity::TakeDamage(BaseEntity* other, int amount, DamageSource source, bool skipImmunity)
{
	if (ImmunityTicks > 0 && !skipImmunity)
		return false;

	if (source != DamageSource::FALL &&
		source != DamageSource::DROWN &&
		source != DamageSource::FIRE)
	{
		int armor = 25 - Inventory.GetArmorValue();
		int reduced = amount * armor;
		Inventory.DamageArmor(amount);
		amount = reduced / 25;
	}
	
	if (BaseEntity::TakeDamage(other, amount, source, skipImmunity))
	{
		EntityWorld->PlaySoundEffect("misc.hurt", 0.3f);
		return true;
	} else if (source == DamageSource::MOB || amount <= 0)
	{
		EntityWorld->PlaySoundEffect("misc.hurt", 0.3f);
		DamageTicks = 10;
		ImmunityTicks = 10;
	}
	return false;
}

float PlayerEntity::GetCameraTilt(float a)
{
	return glm::pow(Interpolate((float)LastDamageTicks, (float)DamageTicks, a) / 10.0f, 2.0f) * 25.0f;
}

void PlayerEntity::OnDeath()
{
	if (EntityWorld->IsServerWorld)
	{
		Inventory.DropAllItems(this, EntityWorld);
	}
	Client::GetInstance().SetScreen(std::make_shared<ScreenGameOver>(Client::GetInstance().GraphicsDevice));
}

void PlayerEntity::Attack(BaseEntity* other, int amount)
{
	if (!other->CanBeHit)
		return;

	int baseDamage = 1;

	auto item = Item::GetItem(Inventory.Resources[HotbarSlot].Id);

	if (item)
	{
		auto sword = dynamic_cast<ItemSword*>(item);
		if (sword)
		{
			baseDamage = sword->Damage;
		}
	}

	if (other->TakeDamage(this, baseDamage, DamageSource::GENERIC))
	{
		if (item)
		{
			if (item->GetType() != TOOL_TYPE_NONE)
				Inventory.Resources[HotbarSlot].IncreaseDamage();
		}
		other->Velocity.y = 0.3f;
		PushEntity(other, 20.0f);
	}
}

void PlayerEntity::DropItemStack(ItemStack& itemStack)
{
	auto& inv = Inventory;
	if (itemStack.Id != 0)
	{
		ItemStack stack{ BLOCK_AIR, 0 };
		stack.Take(itemStack);

		auto item = std::make_shared<ItemEntity>(EntityWorld, Position + GetForward() * 0.5f, stack);
		item->Velocity = GetForward() * 0.35f;
		EntityWorld->AddEntity(item);
	}
}

void PlayerEntity::InteractWith(InteractionType type, int x, int y, int z)
{
	if (type == InteractionType::CRAFTING)
	{
		auto& client = Client::GetInstance();
		client.SetScreen(std::make_shared<ScreenCrafting>(client.GraphicsDevice));
	}
	if (type == InteractionType::FURNACE)
	{
		Client::GetInstance().SetScreen(std::make_shared<ScreenFurnace>(Client::GetInstance().GraphicsDevice, x, y, z));
	}
	if (type == InteractionType::CHEST)
	{
		Client::GetInstance().SetScreen(std::make_shared<ScreenContainer>(Client::GetInstance().GraphicsDevice, x, y, z));
	}
}

int PlayerEntity::GetBlockAtEyePos()
{
	auto pos = glm::ivec3(glm::floor(Position));
	return EntityWorld->GetBlock(pos.x, pos.y, pos.z).Id;
}
