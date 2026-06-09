#include "ItemWheatSeed.h"
#include "ItemReed.h"
#include "ItemFood.h"
#include "ItemBow.h"
#include "ItemDoor.h"
#include "ItemEmptyBucket.h"
#include "ItemBucket.h"

#include "common/Random.h"
#include "engine/Audio.h"

#include "common/World.h"
#include "common/block/Block.h"
#include "common/entity/ItemEntity.h"
#include "common/entity/TNTPrimedEntity.h"

#include <cassert>

Item* Item::GetItem(int id)
{
	if (id >= 256)
	{
		id -= 256;
		return ItemList[id];
	}
	return nullptr;
}

void Item::Init()
{
	IronShovel = (new ItemTool(0, TOOL_TYPE_SHOVEL, TOOL_TIER_IRON))->SetIconIndex(82);
	IronPickaxe = (new ItemTool(1, TOOL_TYPE_PICKAXE, TOOL_TIER_IRON))->SetIconIndex(98);
	IronAxe = (new ItemTool(2, TOOL_TYPE_AXE, TOOL_TIER_IRON))->SetIconIndex(114);

	FlintAndSteel = (new ItemTool(3, TOOL_TYPE_FLINT_AND_STEEL, TOOL_TIER_IRON))->SetIconIndex(5);
	Apple = (new ItemFood(4, 4))->SetIconIndex(10);
	Bow = (new ItemBow(5))->SetIconIndex(21)->SetMaxStackSize(1);
	Arrow = (new Item(6))->SetIconIndex(37);
	Coal = (new Item(7))->SetIconIndex(7);
	Diamond = (new Item(8))->SetIconIndex(55);
	IronIngot = (new Item(9))->SetIconIndex(23);
	GoldIngot = (new Item(10))->SetIconIndex(39);

	IronSword = (new ItemSword(11, TOOL_TIER_IRON))->SetIconIndex(66);

	WoodSword = (new ItemSword(12, TOOL_TIER_WOOD))->SetIconIndex(64);
	WoodShovel = (new ItemTool(13, TOOL_TYPE_SHOVEL, TOOL_TIER_WOOD))->SetIconIndex(80);
	WoodPickaxe = (new ItemTool(14, TOOL_TYPE_PICKAXE, TOOL_TIER_WOOD))->SetIconIndex(96);
	WoodAxe = (new ItemTool(15, TOOL_TYPE_AXE, TOOL_TIER_WOOD))->SetIconIndex(112);

	StoneSword = (new ItemSword(16, TOOL_TIER_STONE))->SetIconIndex(65);
	StoneShovel = (new ItemTool(17, TOOL_TYPE_SHOVEL, TOOL_TIER_STONE))->SetIconIndex(81);
	StonePickaxe = (new ItemTool(18, TOOL_TYPE_PICKAXE, TOOL_TIER_STONE))->SetIconIndex(97);
	StoneAxe = (new ItemTool(19, TOOL_TYPE_AXE, TOOL_TIER_STONE))->SetIconIndex(113);

	DiamondSword = (new ItemSword(20, TOOL_TIER_DIAMOND))->SetIconIndex(67);
	DiamondShovel = (new ItemTool(21, TOOL_TYPE_SHOVEL, TOOL_TIER_DIAMOND))->SetIconIndex(83);
	DiamondPickaxe = (new ItemTool(22, TOOL_TYPE_PICKAXE, TOOL_TIER_DIAMOND))->SetIconIndex(99);
	DiamondAxe = (new ItemTool(23, TOOL_TYPE_AXE, TOOL_TIER_DIAMOND))->SetIconIndex(115);

	Stick = (new Item(24))->SetIconIndex(53);
	BowlEmpty = (new Item(25))->SetIconIndex(71);
	BowlSoup = (new Item(26))->SetIconIndex(72);

	GoldenSword = (new ItemSword(27, TOOL_TIER_GOLD))->SetIconIndex(68);
	GoldenShovel = (new ItemTool(28, TOOL_TYPE_SHOVEL, TOOL_TIER_GOLD))->SetIconIndex(84);
	GoldenPickaxe = (new ItemTool(29, TOOL_TYPE_PICKAXE, TOOL_TIER_GOLD))->SetIconIndex(100);
	GoldenAxe = (new ItemTool(30, TOOL_TYPE_AXE, TOOL_TIER_GOLD))->SetIconIndex(116);

	Silk = (new Item(31))->SetIconIndex(8);
	Feather = (new Item(32))->SetIconIndex(24);
	Gunpowder = (new Item(33))->SetIconIndex(40);

	WoodHoe = (new ItemTool(34, TOOL_TYPE_HOE, TOOL_TIER_WOOD))->SetIconIndex(128);
	StoneHoe = (new ItemTool(35, TOOL_TYPE_HOE, TOOL_TIER_STONE))->SetIconIndex(129);
	IronHoe = (new ItemTool(36, TOOL_TYPE_HOE, TOOL_TIER_IRON))->SetIconIndex(130);
	DiamondHoe = (new ItemTool(37, TOOL_TYPE_HOE, TOOL_TIER_DIAMOND))->SetIconIndex(131);
	GoldHoe = (new ItemTool(38, TOOL_TYPE_HOE, TOOL_TIER_GOLD))->SetIconIndex(132);
	Seeds = (new ItemWheatSeed(39));
	Wheat = (new Item(40))->SetIconIndex(25);
	Bread = (new ItemFood(41, 5))->SetIconIndex(41);

	LeatherHelmet = (new ItemArmor(42, TOOL_TIER_WOOD, 0, TOOL_TYPE_ARMOR_HEAD));
	LeatherPlate = (new ItemArmor(43, TOOL_TIER_WOOD, 16, TOOL_TYPE_ARMOR_CHEST));
	LeatherPants = (new ItemArmor(44, TOOL_TIER_WOOD, 32, TOOL_TYPE_ARMOR_LEGS));
	LeatherBoots = (new ItemArmor(45, TOOL_TIER_WOOD, 48, TOOL_TYPE_ARMOR_FEET));

	ChainHelmet = (new ItemArmor(46, TOOL_TIER_STONE, 1, TOOL_TYPE_ARMOR_HEAD));
	ChainPlate = (new ItemArmor(47, TOOL_TIER_STONE, 17, TOOL_TYPE_ARMOR_CHEST));
	ChainPants = (new ItemArmor(48, TOOL_TIER_STONE, 33, TOOL_TYPE_ARMOR_LEGS));
	ChainBoots = (new ItemArmor(49, TOOL_TIER_STONE, 49, TOOL_TYPE_ARMOR_FEET));

	IronHelmet = (new ItemArmor(50, TOOL_TIER_IRON, 2, TOOL_TYPE_ARMOR_HEAD));
	IronPlate = (new ItemArmor(51, TOOL_TIER_IRON, 18, TOOL_TYPE_ARMOR_CHEST));
	IronPants = (new ItemArmor(52, TOOL_TIER_IRON, 34, TOOL_TYPE_ARMOR_LEGS));
	IronBoots = (new ItemArmor(53, TOOL_TIER_IRON, 50, TOOL_TYPE_ARMOR_FEET));

	DiamondHelmet = (new ItemArmor(54, TOOL_TIER_DIAMOND, 3, TOOL_TYPE_ARMOR_HEAD));
	DiamondPlate = (new ItemArmor(55, TOOL_TIER_DIAMOND, 19, TOOL_TYPE_ARMOR_CHEST));
	DiamondPants = (new ItemArmor(56, TOOL_TIER_DIAMOND, 35, TOOL_TYPE_ARMOR_LEGS));
	DiamondBoots = (new ItemArmor(57, TOOL_TIER_DIAMOND, 51, TOOL_TYPE_ARMOR_FEET));

	GoldHelmet = (new ItemArmor(58, TOOL_TIER_STONE, 4, TOOL_TYPE_ARMOR_HEAD));
	GoldPlate = (new ItemArmor(59, TOOL_TIER_STONE, 20, TOOL_TYPE_ARMOR_CHEST));
	GoldPants = (new ItemArmor(60, TOOL_TIER_STONE, 36, TOOL_TYPE_ARMOR_LEGS));
	GoldBoots = (new ItemArmor(61, TOOL_TIER_STONE, 52, TOOL_TYPE_ARMOR_FEET));

	Flint = (new Item(62))->SetIconIndex(6);
	PorkRaw = (new ItemFood(63, 3))->SetIconIndex(87);
	PorkCooked = (new ItemFood(64, 8))->SetIconIndex(88);

	WoodenDoor = (new ItemDoor(68, BLOCK_WOOD_DOOR_BOTTOM))->SetIconIndex(43);
	Bucket = (new ItemEmptyBucket(69))->SetIconIndex(74);
	WaterBucket = (new ItemBucket(70, BLOCK_WATER))->SetIconIndex(75);
	LavaBucket = (new ItemBucket(71, BLOCK_LAVA))->SetIconIndex(76);

	Leather = (new Item(78))->SetIconIndex(103);
	ClayBrick = (new Item(80))->SetIconIndex(22);
	ClayBalls = (new Item(81))->SetIconIndex(57);
	Reed = (new ItemReed(82));
	Paper = (new Item(83))->SetIconIndex(58);
	Book = (new Item(84))->SetIconIndex(59);

	GlowstoneDust = (new Item(92))->SetIconIndex(73);
}

Item::Item(int id)
{
	m_Id = 256 + id;
	assert(ItemList[id] == NULL);
	ItemList[id] = this;
}

Item* Item::SetMaxStackSize(int size)
{
	m_MaxStackSize = size;
	return this;
}

Item* Item::SetIconIndex(int index)
{
	m_IconIndex = index;
	return this;
}

ItemToolTier Item::GetTier()
{
	return m_Tier;
}

ItemToolType Item::GetType()
{
	return m_Type;
}

int Item::GetMaxDamage()
{
	return m_MaxDamage;
}

int Item::GetMaxStackSize()
{
	return m_MaxStackSize;
}

int Item::GetIconIndex()
{
	return m_IconIndex;
}

unsigned int Item::GetId()
{
	return m_Id;
}

bool Item::TryUseOnLiquid(BaseEntity* entity, World* world, int x, int y, int z, ItemStack& stack)
{
	return false;
}

bool Item::TryUseOnBlock(BaseEntity* entity, World* world, int x, int y, int z)
{
	return false;
}

bool Item::TryPlace(BaseEntity* entity, World* world, int x, int y, int z, ItemStack& stack)
{
	return false;
}

bool Item::TryUseOnEntity(BaseEntity* entity, World* world, int x, int y, int z)
{
	return false;
}

ItemTool::ItemTool(int id, ItemToolType type, ItemToolTier tier) : Item(id)
{
	m_Tier = tier;
	m_Type = type;
	Efficiency = (float)(tier + 1) * 2.0f;
	m_MaxDamage = 32 << tier + 1;

	if (tier == TOOL_TIER_DIAMOND)
	{
		m_MaxDamage *= 4;
	}

	if (tier == TOOL_TIER_GOLD)
	{
		m_Tier = TOOL_TIER_WOOD;
		m_MaxDamage = 32 << TOOL_TIER_WOOD;
		Efficiency = (float)(TOOL_TIER_GOLD + 1) * 3.0f;
	}

	SetMaxStackSize(1);
}

bool ItemTool::TryUseOnBlock(BaseEntity* entity, World* world, int x, int y, int z)
{
	if (m_Type == TOOL_TYPE_FLINT_AND_STEEL)
	{
		auto block = world->GetBlock(x, y, z);
		if (block.Id == BLOCK_TNT)
		{
			world->SetBlock(x, y, z, BLOCK_AIR);
			if (world->IsServerWorld)
			{
				auto entity = std::make_shared<TNTPrimedEntity>(world, x, y, z);
				world->AddEntity(entity);
				world->PlaySound("random.fuse", x + 0.5f, y + 0.5f, z + 0.5f, 0.3f);
			}
			return true;
		}
	}
	if (m_Type == TOOL_TYPE_HOE)
	{
		auto block = world->GetBlock(x, y, z);
		if (block.Id == BLOCK_GRASS || block.Id == BLOCK_DIRT)
		{
			world->SetBlock(x, y, z, BLOCK_FARMLAND);

			world->PlaySound("random.step.gravel", glm::vec3(x, y, z) + glm::vec3(0.5f), 0.2f);

			if (Random::GlobalRand.NextInt(50) < 10 && block.Id == BLOCK_GRASS)
			{
				auto item = std::make_shared<ItemEntity>(world, glm::vec3(x, y + 1, z) + glm::vec3(0.5f), ItemStack{ Seeds->GetId(), 1 });
				world->AddEntity(item);
			}

			return true;
		}
	} 
	return false;
}

bool ItemTool::TryPlace(BaseEntity* entity, World* world, int x, int y, int z, ItemStack& stack)
{
	if (m_Type == TOOL_TYPE_FLINT_AND_STEEL)
	{
		if (world->GetBlock(x, y, z).Id == BLOCK_AIR && Block::GetBlock(BLOCK_FIRE)->CanExistAt(world, x, y, z, -1, -1))
		{
			world->SetBlock(x, y, z, BLOCK_FIRE);
			if (world->IsServerWorld)
				world->PlaySound("misc.ignite", glm::vec3(x, y, z) + glm::vec3(0.5f), 0.15f);
			stack.IncreaseDamage();
			return true;
		}
	}
	return false;
}

bool ItemTool::IsEffectiveVsBlock(int id)
{
	return Block::GetBlock(id)->GetEffectiveToolType() == m_Type && (Block::GetBlock(id)->GetRequiredTier() <= m_Tier || Block::GetBlock(id)->GetRequiredTier()== TOOL_TIER_NONE);
}

float ItemTool::GetEfficiencyVsBlock(int id)
{
	return IsEffectiveVsBlock(id) ? Efficiency : 1.0f;
}

ItemSword::ItemSword(int id, ItemToolTier tier) : Item(id)
{
	SetMaxStackSize(1);

	m_MaxDamage = 32 << tier;

	if (tier == TOOL_TIER_DIAMOND)
	{
		m_MaxDamage *= 4;
	}

	Damage = 4 + (tier * 2);

	if (tier == TOOL_TIER_GOLD)
	{
		m_MaxDamage = 32 << TOOL_TIER_WOOD;
		Damage = 4;
	}

	m_Type = TOOL_TYPE_SWORD;
}

ItemArmor::ItemArmor(int id, ItemToolTier tier, int iconIndex, ItemToolType type) : Item(id)
{
	int index = type - TOOL_TYPE_ARMOR_HEAD;
	static const int protectionLevels[] = { 3, 8, 6, 3 };
	static const int durabilityLevels[] = { 11, 16, 15, 13 };
	ProtectionLevel = protectionLevels[index];
	Level = type;
	m_Type = type;
	m_MaxDamage = durabilityLevels[index] * 3 << tier;
	SetMaxStackSize(1);
	SetIconIndex(iconIndex);
}
