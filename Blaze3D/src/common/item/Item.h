#pragma once

#include "common/ToolTier.h"
#include "common/ItemStack.h"

class BaseEntity;
class World;

class Item
{
public:
	inline static Item* ItemList[256];
	
	static Item* GetItem(int id);

	inline static Item* IronShovel;
	inline static Item* IronPickaxe;
	inline static Item* IronAxe;
	inline static Item* FlintAndSteel;
	inline static Item* Apple;
	inline static Item* Bow;
	inline static Item* Arrow;
	inline static Item* Coal;
	inline static Item* Diamond;
	inline static Item* IronIngot;
	inline static Item* GoldIngot;
	inline static Item* IronSword;
	inline static Item* WoodSword;
	inline static Item* WoodShovel;
	inline static Item* WoodPickaxe;
	inline static Item* WoodAxe;
	inline static Item* StoneSword;
	inline static Item* StoneShovel;
	inline static Item* StonePickaxe;
	inline static Item* StoneAxe;
	inline static Item* DiamondSword;
	inline static Item* DiamondShovel;
	inline static Item* DiamondPickaxe;
	inline static Item* DiamondAxe;
	inline static Item* Silk;
	inline static Item* Feather;
	inline static Item* Gunpowder;
	inline static Item* WoodHoe;
	inline static Item* StoneHoe;
	inline static Item* IronHoe;
	inline static Item* DiamondHoe;
	inline static Item* GoldHoe;
	inline static Item* Seeds;
	inline static Item* Wheat;
	inline static Item* Bread;

	inline static Item* LeatherHelmet;
	inline static Item* LeatherPlate;
	inline static Item* LeatherPants;
	inline static Item* LeatherBoots;

	inline static Item* ChainHelmet;
	inline static Item* ChainPlate;
	inline static Item* ChainPants;
	inline static Item* ChainBoots;

	inline static Item* IronHelmet;
	inline static Item* IronPlate;
	inline static Item* IronPants;
	inline static Item* IronBoots;

	inline static Item* DiamondHelmet;
	inline static Item* DiamondPlate;
	inline static Item* DiamondPants;
	inline static Item* DiamondBoots;

	inline static Item* GoldHelmet;
	inline static Item* GoldPlate;
	inline static Item* GoldPants;
	inline static Item* GoldBoots;

	inline static Item* Flint;
	inline static Item* PorkRaw;
	inline static Item* PorkCooked;
	inline static Item* Stick;
	inline static Item* BowlEmpty;
	inline static Item* BowlSoup;

	inline static Item* GoldenSword;
	inline static Item* GoldenShovel;
	inline static Item* GoldenPickaxe;
	inline static Item* GoldenAxe;

	inline static Item* WoodenDoor;
	inline static Item* Bucket;
	inline static Item* WaterBucket;
	inline static Item* LavaBucket;

	inline static Item* Leather;
	inline static Item* ClayBrick;
	inline static Item* ClayBalls;
	inline static Item* Reed;
	inline static Item* Paper;
	inline static Item* Book;

	inline static Item* GlowstoneDust;

	static void Init();

	Item(int id);

	Item* SetMaxStackSize(int size);
	Item* SetIconIndex(int index);

	ItemToolTier GetTier();
	ItemToolType GetType();

	int GetMaxDamage();
	int GetMaxStackSize();
	int GetIconIndex();
	unsigned int GetId();

	virtual bool TryUseOnLiquid(BaseEntity* entity, World* world, int x, int y, int z, ItemStack& stack);
	virtual bool TryUseOnBlock(BaseEntity* entity, World* world, int x, int y, int z);
	virtual bool TryPlace(BaseEntity* entity, World* world, int x, int y, int z, ItemStack& stack);
	virtual bool TryUseOnEntity(BaseEntity* entity, World* world, int x, int y, int z);

protected:
	int m_MaxStackSize = 64;
	int m_MaxDamage = 32;
	int m_IconIndex = 0;
	int m_Id = 0;
	ItemToolTier m_Tier = TOOL_TIER_NONE;
	ItemToolType m_Type = TOOL_TYPE_NONE;
};

class ItemTool : public Item
{
public:
	float Efficiency = 4.0f;

	ItemTool(int id, ItemToolType type, ItemToolTier tier);

	bool TryUseOnBlock(BaseEntity* entity, World* world, int x, int y, int z);
	bool TryPlace(BaseEntity* entity, World* world, int x, int y, int z, ItemStack& stack);

	bool IsEffectiveVsBlock(int id);
	float GetEfficiencyVsBlock(int id);
};

class ItemSword : public Item
{
public:
	int Damage = 4;

	ItemSword(int id, ItemToolTier tier);
};

class ItemArmor : public Item
{
public:
	int Level = 0;
	int ProtectionLevel = 0;

	ItemArmor(int id, ItemToolTier tier, int iconIndex, ItemToolType type);
};