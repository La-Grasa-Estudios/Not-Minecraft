#include "SPPlayerHandler.h"

#include "common/World.h"
#include "common/item/Item.h"
#include "common/entity/PlayerEntity.h"
#include "common/Random.h"

#include "client/Client.h"
#include "client/ParticleEngine.h"

#include "engine/Audio.h"

static glm::ivec3 hitPosition = {};
static uint8_t hitBlock = 0;
static bool shouldDrop = true;
static int breakTicks = 0;

SPPlayerHandler::SPPlayerHandler(World* world)
{
	m_World = world;
}

void SPPlayerHandler::MainAction(RayCastHit& hit)
{
	auto& client = Client::GetInstance();
	auto Player = client.Player.get();

	Random random;
	if (client.DidHitBlock && m_MineCooldown == 0)
	{
		auto block = m_World->GetBlock(hit.HitBlock.x, hit.HitBlock.y, hit.HitBlock.z).Id;

		auto loc = hit.HitBlock;
		if (hit.Face == 0)
			loc.y -= 1;
		if (hit.Face == 1)
			loc.y += 1;
		if (hit.Face == 2)
			loc.z -= 1;
		if (hit.Face == 3)
			loc.z += 1;
		if (hit.Face == 4)
			loc.x -= 1;
		if (hit.Face == 5)
			loc.x += 1;

		bool extingishedFire = false;

		if (m_World->GetBlock(loc.x, loc.y, loc.z).Id == BLOCK_FIRE)
		{
			m_World->SetBlock(loc.x, loc.y, loc.z, BLOCK_AIR);
			extingishedFire = true;
			AudioEngine::PlaySound("misc.fizz", Player->Position, 0.15f);
			return;
		}

		if (!extingishedFire)
		{
			if (hitPosition != hit.HitBlock || block != hitBlock)
			{
				hitBlock = block;
				hitPosition = hit.HitBlock;

				m_Hardness = Block::blocks[block]->GetHardness();
				float mult = 5.0f;

				if (Block::blocks[block]->GetRequiredTier() == TOOL_TIER_NONE)
				{
					mult = 1.5f;
					shouldDrop = true;
				}
				else
				{
					shouldDrop = false;
				}

				auto& inv = Player->Inventory;
				if (inv.Resources[Player->HotbarSlot].Id != 0)
				{
					auto item = Item::GetItem(inv.Resources[Player->HotbarSlot].Id);
					if (item && item->GetType() != TOOL_TYPE_NONE)
					{
						auto itemTool = static_cast<ItemTool*>(item);
						if (itemTool->IsEffectiveVsBlock(hitBlock))
						{
							mult = 1.5f;
							shouldDrop = true;
						}
						else
						{
							if (Block::GetBlock(block)->GetRequiredTier() != TOOL_TIER_NONE)
							{
								shouldDrop = false;
							}
						}
						m_Hardness /= itemTool->GetEfficiencyVsBlock(hitBlock);
					}
				}

				m_Hardness *= mult;

				m_MineSpeed = m_Hardness;
			}

			float multiplier = 1.0f;

			if (!Player->IsGrounded)
				multiplier /= 5.0f;

			m_Hardness -= (1.0f / 20.0f) * multiplier;

			if (m_Hardness < 0.0f)
			{
				m_Hardness = 0.0f;
				hit.Face = -1;
				if (m_MineSpeed >= 0.05f)
					m_MineCooldown = 6;

				auto& inv = Player->Inventory;
				auto item = Item::GetItem(inv.Resources[Player->HotbarSlot].Id);
				if (item && item->GetType() != TOOL_TYPE_NONE && m_MineSpeed >= 0.05f)
				{
					inv.Resources[Player->HotbarSlot].IncreaseDamage();
				}

				Block::GetBlock(hitBlock)->BreakAt(m_World, Player, hit.HitBlock.x, hit.HitBlock.y, hit.HitBlock.z, shouldDrop);
				Block::GetBlock(hitBlock)->SpawnBreakParticles(m_World, hit.HitBlock.x, hit.HitBlock.y, hit.HitBlock.z);
			}
			else
			{
				if (breakTicks % 4 == 0 && Block::GetBlock(hitBlock)->SoundDef)
				{
					AudioEngine::PlaySound(Block::GetBlock(hitBlock)->SoundDef->DigSound, hit.HitBlock, 0.1f);
				}
				breakTicks++;

				auto bb = m_World->GetBlockBoundingBox(hit.HitBlock.x, hit.HitBlock.y, hit.HitBlock.z);

				float xp = bb.minX;
				float yp = bb.minY;
				float zp = bb.minZ;

				float ya = 0.5f;

				if (hit.Face == 0) {
					yp = bb.minY - 0.1f;
					xp = glm::mix(bb.minX, bb.maxX, random.NextFloat());
					zp = glm::mix(bb.minZ, bb.maxZ, random.NextFloat());
					ya = -0.1f;
				}
				else if (hit.Face == 1) {
					yp = bb.maxY + 0.1f;
					xp = glm::mix(bb.minX, bb.maxX, random.NextFloat());
					zp = glm::mix(bb.minZ, bb.maxZ, random.NextFloat());
				}
				else if (hit.Face == 2) {
					zp = bb.minZ - 0.1f;
					xp = glm::mix(bb.minX, bb.maxX, random.NextFloat());
					yp = glm::mix(bb.minY, bb.maxY, random.NextFloat());
				}
				else if (hit.Face == 3) {
					zp = bb.maxZ + 0.1f;
					xp = glm::mix(bb.minX, bb.maxX, random.NextFloat());
					yp = glm::mix(bb.minY, bb.maxY, random.NextFloat());
				}
				else if (hit.Face == 4) {
					xp = bb.minX - 0.1f;
					zp = glm::mix(bb.minZ, bb.maxZ, random.NextFloat());
					yp = glm::mix(bb.minY, bb.maxY, random.NextFloat());
				}
				else if (hit.Face == 5) {
					xp = bb.maxX + 0.1f;
					zp = glm::mix(bb.minZ, bb.maxZ, random.NextFloat());
					yp = glm::mix(bb.minY, bb.maxY, random.NextFloat());
				}

				client.ParticleManager->InitTerrainParticle(xp, yp, zp, (random.NextFloat() * 2.0f - 1.0f) * 0.2f, ya + (random.NextFloat() * 2.0f - 1.0f) * 0.3f, (random.NextFloat() * 2.0f - 1.0f) * 0.2f, 0.3f, hitBlock);
				m_IsBreaking = true;
			}
		}
	}
	else
	{
		hitBlock = 0;
		hitPosition = {};
		m_Hardness = 0;
		breakTicks = 0;
	}
}

void SPPlayerHandler::MainActionDown(RayCastHit& hit, BaseEntity* hitEntity)
{

}

void SPPlayerHandler::SecondaryAction(RayCastHit& hit)
{
	auto& client = Client::GetInstance();
	auto Player = client.Player.get();

	bool didSecondary = false;

	RayCastHit liquidHit;
	bool didHitLiquid = m_World->RayCastLiquids(Player->Position, Player->GetForward(), 5.0f, liquidHit);

	if (client.DidHitBlock)
	{
		// If true means block can be right clicked
		bool used = false;
		auto& inv = Player->Inventory;
		if (m_PlaceCooldown == 0)
		{
			used = m_World->GetBlock(hit.HitBlock.x, hit.HitBlock.y, hit.HitBlock.z).GetDef()->UseAt(m_World, Player, hit.HitBlock.x, hit.HitBlock.y, hit.HitBlock.z);
			if (used) didSecondary = true;
		}
		if (!used)
		{
			auto& itemStack = inv.Resources[Player->HotbarSlot];
			if (itemStack.Id != 0 && itemStack.Id < 256 && itemStack.Amount > 0 && m_PlaceCooldown == 0)
			{
				auto loc = hit.HitBlock;

				auto toPlace = Block::GetBlock(itemStack.Id);
				bool occupied = false;

				bool PlacedSlab = false;
				int id = itemStack.Id;

				if (hit.Face == 1 && m_World->GetBlock(loc.x, loc.y, loc.z).Id == id && (id == BLOCK_STONE_SLAB || id == BLOCK_WOOD_SLAB || id == BLOCK_COBBLESTONE_SLAB))
				{
					AABB bbs[4];
					int count = 0;

					auto lastBlock = m_World->GetBlock(loc.x, loc.y, loc.z);

					m_World->SetBlock(loc.x, loc.y, loc.z, id - 3);

					count = Block::GetBlock(id - 3)->GetCollisionBoundingBoxes(m_World, loc.x, loc.y, loc.z, bbs);

					for (int i = 0; i < count && !occupied; i++)
					{
						int count = 0;
						auto entities = m_World->GetCollidingEntitiesWith(bbs[i], &count);
						for (int j = 0; j < count; j++)
						{
							if (entities[j]->CanBeRaycasted)
							{
								occupied = true;
								break;
							}
						}
					}

					if (occupied)
					{
						m_World->SetBlock(loc.x, loc.y, loc.z, lastBlock);
					}
					else
					{
						PlacedSlab = true;
						itemStack.Shrink(1);
						m_PlaceCooldown = 5;
						Block::GetBlock(itemStack.Id)->OnPlace(m_World, Player, loc.x, loc.y, loc.z, 0, 0);
						client.IngameUI.DoPlaceSwing = true;

						if (Block::GetBlock(itemStack.Id)->SoundDef)
						{
							AudioEngine::PlaySound(Block::GetBlock(itemStack.Id)->SoundDef->StepSound, 0.25f);
						}
						didSecondary = true;
						return;
					}
				}

				auto existing = m_World->GetBlock(loc.x, loc.y, loc.z).GetDef();

				if (hit.Face == 0)
					loc.y -= 1;
				if (hit.Face == 1)
					loc.y += 1;
				if (hit.Face == 2)
					loc.z -= 1;
				if (hit.Face == 3)
					loc.z += 1;
				if (hit.Face == 4)
					loc.x -= 1;
				if (hit.Face == 5)
					loc.x += 1;

				if (existing->IsReplaceableByPlayer())
				{
					loc = hit.HitBlock;
				}

				if (!PlacedSlab && m_World->GetBlock(loc.x, loc.y, loc.z).GetDef()->IsReplaceableByPlayer() && toPlace->CanExistAt(m_World, loc.x, loc.y, loc.z, hit.Face, 0))
				{
					auto lastBlock = m_World->GetBlock(loc.x, loc.y, loc.z);

					m_World->SetBlock(loc.x, loc.y, loc.z, itemStack.Id);

					AABB bbs[4];
					int count = Block::GetBlock(itemStack.Id)->GetCollisionBoundingBoxes(m_World, loc.x, loc.y, loc.z, bbs);
					Block::GetBlock(itemStack.Id)->OnPlace(m_World, Player, loc.x, loc.y, loc.z, hit.Face, Player->GetEntityFacingDirection());

					for (int i = 0; i < count && !occupied; i++)
					{
						int count = 0;
						auto entities = m_World->GetCollidingEntitiesWith(bbs[i], &count);
						for (int j = 0; j < count; j++)
						{
							if (entities[j]->CanBeRaycasted)
							{
								occupied = true;
								break;
							}
						}
					}

					if (!occupied)
					{
						if (Block::GetBlock(itemStack.Id)->SoundDef)
						{
							AudioEngine::PlaySound(Block::GetBlock(itemStack.Id)->SoundDef->StepSound, 0.25f);
						}

						itemStack.Shrink(1);
						client.IngameUI.DoPlaceSwing = true;
						m_PlaceCooldown = 5;
						didSecondary = true;
					}
					else
					{
						m_World->SetBlock(loc.x, loc.y, loc.z, lastBlock);
					}
				}
			}
			else if (m_PlaceCooldown == 0)
			{
				auto item = Item::GetItem(itemStack.Id);
				if (item)
				{
					auto loc = hit.HitBlock;
					if (hit.Face == 0)
						loc.y -= 1;
					if (hit.Face == 1)
						loc.y += 1;
					if (hit.Face == 2)
						loc.z -= 1;
					if (hit.Face == 3)
						loc.z += 1;
					if (hit.Face == 4)
						loc.x -= 1;
					if (hit.Face == 5)
						loc.x += 1;
					if (item->TryUseOnBlock(Player, m_World, hit.HitBlock.x, hit.HitBlock.y, hit.HitBlock.z))
					{
						m_PlaceCooldown = 5;
						client.IngameUI.DoPlaceSwing = true;
						itemStack.IncreaseDamage();
						didSecondary = true;
					}
					else if (!m_World->GetBlock(loc.x, loc.y, loc.z).GetDef()->IsSolidToRaycast() && item->TryPlace(Player, m_World, loc.x, loc.y, loc.z, itemStack))
					{
						client.IngameUI.DoPlaceSwing = true;
						m_PlaceCooldown = 5;
						didSecondary = true;
					}
				}
			}
		}
		else
		{
			m_PlaceCooldown = 5;
			client.IngameUI.DoDigSwing = true;
		}

	}

	if (didHitLiquid && !didSecondary)
	{
		auto item = Item::GetItem(Player->Inventory.Resources[Player->HotbarSlot].Id);
		if (item && m_PlaceCooldown == 0)
		{
			if (item->TryUseOnLiquid(Player, m_World, liquidHit.HitBlock.x, liquidHit.HitBlock.y, liquidHit.HitBlock.z, Player->Inventory.Resources[Player->HotbarSlot]))
			{
				m_PlaceCooldown = 5;
			}
		}
	}
}

void SPPlayerHandler::SecondaryActionDown(RayCastHit& hit, BaseEntity* hitEntity)
{

}

void SPPlayerHandler::MainActionUnPressed()
{
	m_IsBreaking = false;
	m_MineSpeed = 1.0f;
	m_Hardness = 0.0f;
	hitBlock = 0;
	hitPosition = {};
	m_Hardness = 0;
	breakTicks = 0;
}

void SPPlayerHandler::SecondaryActionUnPressed()
{
	m_PlaceCooldown = 0;
}

void SPPlayerHandler::Tick()
{
	if (m_PlaceCooldown != 0)
	{
		m_PlaceCooldown--;
	}
	if (m_MineCooldown > 0)
	{
		m_MineCooldown--;
	}
}

float SPPlayerHandler::GetDigProgress()
{
	if (!m_IsBreaking)
	{
		return 0.0f;
	}
	return m_Hardness / m_MineSpeed;
}
