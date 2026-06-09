#pragma once

#include <cstdint>
#include <string>

#include "common/ToolTier.h"

class World;
struct Chunk;
class VertexProducer;
class BaseEntity;
struct AABB;

struct BlockSoundDef
{
	std::string DigSound;
	std::string StepSound;
};

enum BlockName : uint8_t
{
	BLOCK_AIR,
	BLOCK_STONE,
	BLOCK_GRASS,
	BLOCK_DIRT,
	BLOCK_COBBLESTONE,
	BLOCK_PLANKS,
	BLOCK_SAPLING,
	BLOCK_BEDROCK,
	BLOCK_WATER,
	BLOCK_LAVA,
	BLOCK_SAND,
	BLOCK_GRAVEL,
	BLOCK_GOLD_ORE,
	BLOCK_IRON_ORE,
	BLOCK_COAL_ORE,
	BLOCK_LOG,
	BLOCK_LEAVES,
	BLOCK_SPONGE,
	BLOCK_GLASS,
	BLOCK_RED_CLOTH,
	BLOCK_ORANGE_CLOTH,
	BLOCK_YELLOW_CLOTH,
	BLOCK_LIME_CLOTH,
	BLOCK_GREEN_CLOTH,
	BLOCK_AQUA_GREEN_CLOTH,
	BLOCK_CYAN_CLOTH,
	BLOCK_BLUE_CLOTH,
	BLOCK_PURPLE_CLOTH,
	BLOCK_INDIGO_CLOTH,
	BLOCK_VIOLET_CLOTH,
	BLOCK_MAGENTA_CLOTH,
	BLOCK_PINK_CLOTH,
	BLOCK_BLACK_CLOTH,
	BLOCK_GRAY_CLOTH,
	BLOCK_WHITE_CLOTH,
	BLOCK_YELLOW_FLOWER,
	BLOCK_RED_ROSE,
	BLOCK_BROWN_MUSHROOM,
	BLOCK_RED_MUSHROOM,
	BLOCK_IRON,
	BLOCK_GOLD,
	BLOCK_DOUBLE_STONE_SLAB,
	BLOCK_DOUBLE_WOOD_SLAB,
	BLOCK_DOUBLE_COBBLESTONE_SLAB,
	BLOCK_STONE_SLAB,
	BLOCK_WOOD_SLAB,
	BLOCK_COBBLESTONE_SLAB,
	BLOCK_BRICK,

	BLOCK_MOSSY_COBBLESTONE,
	BLOCK_OBSIDIAN,
	BLOCK_TORCH,

	BLOCK_CRAFTING_TABLE,
	BLOCK_WHEAT_CROPS,
	BLOCK_FARMLAND,
	BLOCK_FURNACE,
	BLOCK_CHEST,

	BLOCK_DIAMOND_ORE,
	BLOCK_DIAMOND,

	BLOCK_WOOD_STAIRS,
	BLOCK_COBBLESTONE_STAIRS,

	BLOCK_BURNING_FURNACE,

	BLOCK_BOOKSHELF,
	BLOCK_SNOW = 78,
	BLOCK_CACTUS = 81,
	BLOCK_CLAY = 82,
	BLOCK_REED = 83,
	BLOCK_FENCE = 84,
	BLOCK_JUKEBOX = 85,

	// v1.2.0 shit
	BLOCK_PUMPKIN,
	BLOCK_NETHERRACK,
	BLOCK_SOUL_SAND,
	BLOCK_GLOWSTONE,
	BLOCK_PORTAL,
	BLOCK_JACK_O_LANTERN,
	BLOCK_FIRE,

	BLOCK_TNT,
	BLOCK_WOOD_DOOR_BOTTOM,
	BLOCK_WOOD_DOOR_TOP,

	BLOCK_VOID = 255,
};

enum BlockLiquidType
{
	LIQUID_TYPE_NONE,
	LIQUID_TYPE_WATER,
	LIQUID_TYPE_LAVA,
};

enum BlockRenderType
{
	BLOCK_RENDER_TYPE_GENERIC,
	BLOCK_RENDER_TYPE_GRASS_BLOCK,
	BLOCK_RENDER_TYPE_CROSS,
	BLOCK_RENDER_TYPE_DOOR,
	BLOCK_RENDER_TYPE_FIRE,
	BLOCK_RENDER_TYPE_LIQUID,
	BLOCK_RENDER_TYPE_PORTAL,
	BLOCK_RENDER_TYPE_STAIRS,
	BLOCK_RENDER_TYPE_TORCH,
	BLOCK_RENDER_TYPE_CROP,
};

class BlockRenderer;
class BlockPreviewRenderer;

class Block
{
public:

	friend BlockRenderer;
	friend BlockPreviewRenderer;

	static Block* blocks[128];
	static void InitBlocks();
	static Block* GetBlock(uint8_t id);

	Block() = default;
	Block(uint8_t id, uint8_t texture);

	virtual BlockRenderType GetRenderType();

	virtual bool IsFaceRenderable(Chunk* chunk, int x, int y, int z, int face);
	virtual uint8_t GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z);

	virtual void OnPlace(World* world, BaseEntity* entity, int x, int y, int z, int face, int facing);
	virtual void BreakAt(World* world, BaseEntity* entity, int x, int y, int z, bool doDrop = true);
	virtual void SpawnBreakParticles(World* world, int x, int y, int z);
	virtual void DropAsItemWithChance(World* world, int x, int y, int z);
	virtual void OnBlockExploded(World* world, int x, int y, int z);
	virtual bool UseAt(World* world, BaseEntity* entity, int x, int y, int z);
	virtual bool CanExistAt(World* world, int x, int y, int z, int faceId, int facing);

	virtual void Tick(World* world, int x, int y, int z);
	virtual void FluidTick(World* world, int x, int y, int z);
	virtual void RandomTick(World* world, int x, int y, int z);
	virtual void DisplayTick(World* world, int x, int y, int z);

	virtual AABB GetWorldBoundingBox(World* world, int x, int y, int z);
	virtual AABB GetRaycastBoundingBox(World* world, int x, int y, int z);
	virtual int GetCollisionBoundingBoxes(World* world, int x, int y, int z, AABB bbs[4]);

	virtual int GetLiquidType() { return 0; }

	Block* SetRenderNeighbours(bool val);
	bool GetRenderNeighbours() const;

	Block* SetColorizeWithBiome(bool val);
	Block* SetTint(float r, float g, float b);
	Block* SetHardness(float hardness);
	float GetHardness() const;

	Block* SetIsFlammable(bool val, int ability, int chance);
	Block* SetIsSolid(bool val);
	Block* SetIsOpaque(bool val);
	Block* SetIsSolidToRaycast(bool val);
	Block* SetIsReplaceable(bool val);
	Block* SetIsReplaceableByPlayer(bool val);
	Block* SetRenderAsIcon(bool val);
	Block* SetRendersToItself(bool val);
	Block* SetCreatesBreakParticles(bool val);
	Block* SetDrop(uint32_t id);
	Block* SetDropCount(uint32_t count);
	Block* SetIconIndex(uint32_t index);
	Block* SetSoundDef(BlockSoundDef* def);
	Block* SetLayer(uint8_t layer);
	Block* SetOpacity(uint8_t op);
	Block* SetLightEmitted(uint8_t level);
	Block* SetShape(float mx, float my, float mz, float xx, float xy, float xz);
	Block* SetMakesSound(bool val);
	Block* SetEffectiveTool(ItemToolTier tier, ItemToolType type);
	Block* SetResistance(float resistance);

	uint8_t GetRenderLayer() const;
	uint8_t GetId() const;
	uint8_t GetOpacity() const;
	uint8_t GetLightEmitted() const;
	uint8_t GetIconIndex() const;
	bool IsFlammable() const;
	bool IsSolid() const;
	bool IsOpaque() const;
	bool IsSolidToRaycast() const;
	bool IsReplaceable() const;
	bool IsReplaceableByPlayer() const;
	bool BlocksLightToTheSides() const;
	bool MakesSound() const;
	bool RendersAsIcon() const;
	bool IsFullyOpaqueBlock() const;
	float GetBlockResistance(BaseEntity* entity) const;

	int GetAbilityToCatchFire() const;
	int GetChanceOfFire() const;

	ItemToolTier GetRequiredTier() const;
	ItemToolType GetEffectiveToolType() const;

	BlockSoundDef* SoundDef;

	float MinX = 0.0f;
	float MaxX = 1.0f;
	float MinY = 0.0f;
	float MaxY = 1.0f;
	float MinZ = 0.0f;
	float MaxZ = 1.0f;

protected:

	virtual int GetDropId();
	virtual int GetDropCount();

	Block* SetTexture(uint8_t x, uint8_t y);

	float CalculateLightLevelForFace(Chunk* chunk, int x, int y, int z, int face);

	bool m_RenderNeighbours = false;
	bool m_RenderToItself = true;
	bool m_RenderAsIcon = false;
	bool m_IsSolid = true;
	bool m_IsOpaque = true;
	bool m_BlocksSideLight = true;
	bool m_IsSolidToRaycast = true;
	bool m_IsReplaceable = false;
	bool m_MakesSound = true;
	bool m_CreatesBreakParticles = true;
	bool m_ReplaceableByPlayer = false;
	bool m_ColorizeWithBiome = false;
	bool m_IsFlammable = false;
	bool m_FlipTextureForRender = false;

	int m_AbilityToCatchFire = 0;
	int m_ChanceToEncourageFire = 0;	

	uint32_t m_ItemDropId;
	uint32_t m_ItemDropCount = 1;
	uint8_t m_Id;
	uint8_t m_TextureIndex;
	uint8_t m_IconIndex = 0;
	uint8_t m_MeshLayer = 0;
	uint8_t m_Opacity = 15;
	uint8_t m_LightEmitted = 0;

	ItemToolTier m_RequiredTier;
	ItemToolType m_EffectiveTool;

	float m_TintR = 1.0f, m_TintG = 1.0f, m_TintB = 1.0f;
	float m_Hardness = 1.0f;
	float m_ExplosionResistance = 5.0f;
};

struct BlockLightStorage
{
	uint8_t SkyLight : 4;
	uint8_t BlockLight : 4;
};

struct BlockStorage
{
	uint8_t Id;
	uint8_t Metadata;

	Block* GetDef();
	BlockStorage() = default;
	BlockStorage(uint8_t id) : Id(id), Metadata(0) {}
	BlockStorage(uint8_t id, uint8_t meta) : Id(id), Metadata(meta) {}
};