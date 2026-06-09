#include "BlockGrass.h"
#include "BlockLog.h"
#include "BlockFallingSand.h"
#include "BlockLiquid.h"
#include "BlockCraftingTable.h"
#include "BlockCrossShaped.h"
#include "BlockFlower.h"
#include "BlockFungus.h"
#include "BlockSlab.h"
#include "BlockTorch.h"
#include "BlockFarmland.h"
#include "BlockCrop.h"
#include "BlockFurnace.h"
#include "BlockChest.h"
#include "BlockStairs.h"
#include "BlockPumpkin.h"
#include "BlockReed.h"
#include "BlockBookshelf.h"
#include "BlockCactus.h"
#include "BlockNetherPortal.h"
#include "BlockFire.h"
#include "BlockLeaves.h"
#include "BlockGravel.h"
#include "BlockTNT.h"
#include "BlockDoor.h"

#include "common/World.h"
#include "common/entity/ItemEntity.h"
#include "common/world/ChunkProvider.h"

#include "engine/Audio.h"
#include "common/Random.h"

#include <string>

Block* Block::blocks[128];

void LoadSounds(const std::string name, const std::string path, const std::string format, int count, std::string* dest)
{
    *dest = name;
    for (int i = 0; i < count; i++)
    {
        auto newPath = path + std::to_string(i + 1) + format;
        AudioEngine::RegisterSound(name, newPath);
    }
}

Block* BlockStorage::GetDef()
{
	return Block::blocks[Id];
}

void Block::InitBlocks()
{
    BlockSoundDef* grassDef = new BlockSoundDef();
    BlockSoundDef* gravelDef = new BlockSoundDef();
    BlockSoundDef* stoneDef = new BlockSoundDef();
    BlockSoundDef* woodDef = new BlockSoundDef();
    BlockSoundDef* clothDef = new BlockSoundDef();
    LoadSounds("random.dig.grass", "mc/sound/dig/grass", ".wav", 6, &grassDef->DigSound);
    LoadSounds("random.step.grass", "mc/sound/step/grass", ".wav", 6, &grassDef->StepSound);
    LoadSounds("random.dig.gravel", "mc/sound/dig/gravel", ".wav", 4, &gravelDef->DigSound);
    LoadSounds("random.step.gravel", "mc/sound/step/gravel", ".wav", 4, &gravelDef->StepSound);
    LoadSounds("random.dig.stone", "mc/sound/dig/stone", ".wav", 6, &stoneDef->DigSound);
    LoadSounds("random.step.stone", "mc/sound/step/stone", ".wav", 6, &stoneDef->StepSound);
    LoadSounds("random.dig.wood", "mc/sound/dig/wood", ".wav", 6, &woodDef->DigSound);
    LoadSounds("random.step.wood", "mc/sound/step/wood", ".wav", 6, &woodDef->StepSound);
    LoadSounds("random.dig.cloth", "mc/sound/dig/cloth", ".wav", 4, &clothDef->DigSound);
    LoadSounds("random.step.cloth", "mc/sound/step/cloth", ".wav", 4, &clothDef->StepSound);

    memset(blocks, 0, sizeof(blocks));

    (new Block(BLOCK_AIR, 0))
        ->SetMakesSound(false)
        ->SetOpacity(0)
        ->SetSoundDef(stoneDef)
        ->SetCreatesBreakParticles(false)
        ->SetIsSolidToRaycast(false)
        ->SetIsSolid(false)
        ->SetIsReplaceable(true)
        ->SetIsReplaceableByPlayer(true)
        ->SetRenderNeighbours(true);

    (new Block(BLOCK_STONE, 1))
        ->SetEffectiveTool(TOOL_TIER_WOOD, TOOL_TYPE_PICKAXE)
        ->SetDrop(BLOCK_COBBLESTONE)
        ->SetHardness(1.5f)
        ->SetResistance(10.0f);

    (new BlockGrass(BLOCK_GRASS))
        ->SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_SHOVEL)
        ->SetSoundDef(grassDef)
        ->SetHardness(0.6f);

    (new Block(BLOCK_DIRT, 2))
        ->SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_SHOVEL)
        ->SetSoundDef(gravelDef)
        ->SetHardness(0.5f);

    (new Block(BLOCK_COBBLESTONE, 16))
        ->SetEffectiveTool(TOOL_TIER_WOOD, TOOL_TYPE_PICKAXE)
        ->SetHardness(2.0f)
        ->SetResistance(10.0f);

    (new BlockFallingSand(BLOCK_SAND, 18))
        ->SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_SHOVEL)
        ->SetSoundDef(gravelDef)
        ->SetHardness(0.5f);

    (new BlockGravel(BLOCK_GRAVEL, 19))
        ->SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_SHOVEL)
        ->SetSoundDef(gravelDef)
        ->SetHardness(0.5f);

    (new Block(BLOCK_PLANKS, 4))
        ->SetIsFlammable(true, 5, 20)
        ->SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_AXE)
        ->SetSoundDef(woodDef)
        ->SetHardness(2.0f)
        ->SetResistance(5.0f);

    (new BlockLog(BLOCK_LOG))
        ->SetIsFlammable(true, 5, 5)
        ->SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_AXE)
        ->SetSoundDef(woodDef)
        ->SetHardness(2.0f);

    (new BlockLeaves(BLOCK_LEAVES, 52)) // Leaves
        ->SetIsFlammable(true, 30, 60)
        ->SetSoundDef(grassDef)
        ->SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_HOE)
        ->SetOpacity(3)
        ->SetDrop(0)
        ->SetHardness(0.2f)
        ->SetRenderNeighbours(true)
        ->SetColorizeWithBiome(true);

    (new BlockLiquid(BLOCK_WATER, LIQUID_TYPE_WATER));
    (new BlockLiquid(BLOCK_LAVA, LIQUID_TYPE_LAVA));

    (new BlockCraftingTable(BLOCK_CRAFTING_TABLE))
        ->SetSoundDef(woodDef);

    (new BlockCrossShaped(BLOCK_SAPLING))
        ->SetTexture(15, 0)
        ->SetSoundDef(grassDef);

    (new Block(BLOCK_BEDROCK, 17))
        ->SetHardness(999999999.0f);

    (new Block(BLOCK_GOLD_ORE, 32))
        ->SetEffectiveTool(TOOL_TIER_IRON, TOOL_TYPE_PICKAXE)
        ->SetHardness(3.0f)
        ->SetResistance(5.0f);

    (new Block(BLOCK_IRON_ORE, 33))
        ->SetEffectiveTool(TOOL_TIER_STONE, TOOL_TYPE_PICKAXE)
        ->SetHardness(3.0f)
        ->SetResistance(5.0f);

    (new Block(BLOCK_DIAMOND_ORE, 50))
        ->SetDrop(264)
        ->SetEffectiveTool(TOOL_TIER_IRON, TOOL_TYPE_PICKAXE)
        ->SetHardness(3.0f)
        ->SetResistance(5.0f);

    (new Block(BLOCK_COAL_ORE, 34))
        ->SetEffectiveTool(TOOL_TIER_WOOD, TOOL_TYPE_PICKAXE)
        ->SetDrop(263)
        ->SetHardness(3.0f)
        ->SetResistance(5.0f);

    (new Block(BLOCK_SPONGE, 48))
        ->SetSoundDef(grassDef)
        ->SetHardness(0.6f);

    (new Block(BLOCK_GLASS, 49))
        ->SetRenderNeighbours(true)
        ->SetRendersToItself(false)
        ->SetOpacity(0)
        ->SetHardness(0.3f);

    auto clothHardness = 0.8f;

    (new Block(BLOCK_RED_CLOTH, 64))
        ->SetIsFlammable(true, 30, 60)
        ->SetTint(0.631f, 0.153f, 0.133f)
        ->SetSoundDef(clothDef)
        ->SetHardness(clothHardness);

    (new Block(BLOCK_ORANGE_CLOTH, 64))
        ->SetIsFlammable(true, 30, 60)
        ->SetTint(0.941f, 0.463f, 0.074f)
        ->SetSoundDef(clothDef)
        ->SetHardness(clothHardness);

    (new Block(BLOCK_YELLOW_CLOTH, 64))
        ->SetIsFlammable(true, 30, 60)
        ->SetTint(0.973f, 0.776f, 0.153f)
        ->SetSoundDef(clothDef)
        ->SetHardness(clothHardness);

    (new Block(BLOCK_LIME_CLOTH, 64))
        ->SetIsFlammable(true, 30, 60)
        ->SetTint(0.439f, 0.725f, 0.098f)
        ->SetSoundDef(clothDef)
        ->SetHardness(clothHardness);

    (new Block(BLOCK_GREEN_CLOTH, 64))
        ->SetIsFlammable(true, 30, 60)
        ->SetTint(0.329f, 0.427f, 0.106f)
        ->SetSoundDef(clothDef)
        ->SetHardness(clothHardness);

    (new Block(BLOCK_AQUA_GREEN_CLOTH, 64))
        ->SetIsFlammable(true, 30, 60)
        ->SetTint(0.082f, 0.537f, 0.569f)
        ->SetSoundDef(clothDef)
        ->SetHardness(clothHardness);

    (new Block(BLOCK_CYAN_CLOTH, 64))
        ->SetIsFlammable(true, 30, 60)
        ->SetTint(0.082f, 0.537f, 0.569f)
        ->SetSoundDef(clothDef)
        ->SetHardness(clothHardness);

    (new Block(BLOCK_BLUE_CLOTH, 64))
        ->SetIsFlammable(true, 30, 60)
        ->SetTint(0.208f, 0.224f, 0.616f)
        ->SetSoundDef(clothDef)
        ->SetHardness(clothHardness);

    (new Block(BLOCK_PURPLE_CLOTH, 64))
        ->SetIsFlammable(true, 30, 60)
        ->SetTint(0.475f, 0.165f, 0.675f)
        ->SetSoundDef(clothDef)
        ->SetHardness(clothHardness);

    (new Block(BLOCK_INDIGO_CLOTH, 64))
        ->SetIsFlammable(true, 30, 60)
        ->SetTint(0.294f, 0.0f, 0.510f)
        ->SetSoundDef(clothDef)
        ->SetHardness(clothHardness);

    (new Block(BLOCK_VIOLET_CLOTH, 64))
        ->SetIsFlammable(true, 30, 60)
        ->SetTint(0.561f, 0.0f, 1.0f)
        ->SetSoundDef(clothDef)
        ->SetHardness(clothHardness);

    (new Block(BLOCK_MAGENTA_CLOTH, 64))
        ->SetIsFlammable(true, 30, 60)
        ->SetTint(0.741f, 0.267f, 0.702f)
        ->SetSoundDef(clothDef)
        ->SetHardness(clothHardness);

    (new Block(BLOCK_PINK_CLOTH, 64))
        ->SetIsFlammable(true, 30, 60)
        ->SetTint(0.929f, 0.553f, 0.675f)
        ->SetSoundDef(clothDef)
        ->SetHardness(clothHardness);

    (new Block(BLOCK_BLACK_CLOTH, 64))
        ->SetIsFlammable(true, 30, 60)
        ->SetTint(0.078f, 0.082f, 0.098f)
        ->SetSoundDef(clothDef)
        ->SetHardness(clothHardness);

    (new Block(BLOCK_GRAY_CLOTH, 64))
        ->SetIsFlammable(true, 30, 60)
        ->SetTint(0.243f, 0.267f, 0.278f)
        ->SetSoundDef(clothDef)
        ->SetHardness(clothHardness);

    (new Block(BLOCK_WHITE_CLOTH, 64))
        ->SetIsFlammable(true, 30, 60)
        ->SetTint(0.914f, 0.925f, 0.925f)
        ->SetSoundDef(clothDef)
        ->SetHardness(clothHardness);

    (new BlockFlower(BLOCK_YELLOW_FLOWER, 13))
        ->SetSoundDef(grassDef);

    (new BlockFlower(BLOCK_RED_ROSE, 12))
        ->SetSoundDef(grassDef);

    (new BlockFungus(BLOCK_RED_MUSHROOM, 29))
        ->SetSoundDef(grassDef);

    (new BlockFungus(BLOCK_BROWN_MUSHROOM, 28))
        ->SetSoundDef(grassDef);

    (new Block(BLOCK_IRON, 22))
        ->SetEffectiveTool(TOOL_TIER_STONE, TOOL_TYPE_PICKAXE)
        ->SetHardness(5.0f)
        ->SetResistance(10.0f);

    (new Block(BLOCK_GOLD, 23))
        ->SetEffectiveTool(TOOL_TIER_IRON, TOOL_TYPE_PICKAXE)
        ->SetHardness(5.0f)
        ->SetResistance(10.0f);

    (new Block(BLOCK_DIAMOND, 24))
        ->SetEffectiveTool(TOOL_TIER_IRON, TOOL_TYPE_PICKAXE)
        ->SetHardness(5.0f)
        ->SetResistance(10.0f);

    (new Block(BLOCK_DOUBLE_STONE_SLAB, 5))
        ->SetDrop(BLOCK_STONE_SLAB)
        ->SetDropCount(2)
        ->SetEffectiveTool(TOOL_TIER_WOOD, TOOL_TYPE_PICKAXE)
        ->SetHardness(2.0f)
        ->SetResistance(10.0f);

    (new Block(BLOCK_DOUBLE_WOOD_SLAB, 4))
        ->SetDrop(BLOCK_WOOD_SLAB)
        ->SetDropCount(2)
        ->SetEffectiveTool(TOOL_TIER_WOOD, TOOL_TYPE_PICKAXE)
        ->SetHardness(2.0f)
        ->SetResistance(10.0f);

    (new Block(BLOCK_DOUBLE_COBBLESTONE_SLAB, 16))
        ->SetDrop(BLOCK_COBBLESTONE_SLAB)
        ->SetDropCount(2)
        ->SetEffectiveTool(TOOL_TIER_WOOD, TOOL_TYPE_PICKAXE)
        ->SetHardness(2.0f)
        ->SetResistance(10.0f);

    (new BlockSlab(BLOCK_STONE_SLAB, 6, 5))
        ->SetResistance(10.0f);
    (new BlockSlab(BLOCK_WOOD_SLAB, 4, 4))
        ->SetResistance(10.0f);
    (new BlockSlab(BLOCK_COBBLESTONE_SLAB, 16, 16))
        ->SetResistance(10.0f);

    (new Block(BLOCK_BRICK, 7))
        ->SetEffectiveTool(TOOL_TIER_WOOD, TOOL_TYPE_PICKAXE)
        ->SetHardness(2.0f)
        ->SetResistance(10.0f);

    (new Block(BLOCK_MOSSY_COBBLESTONE, 36))
        ->SetEffectiveTool(TOOL_TIER_WOOD, TOOL_TYPE_PICKAXE)
        ->SetHardness(2.0f)
        ->SetResistance(10.0f);

    (new Block(BLOCK_OBSIDIAN, 37))
        ->SetEffectiveTool(TOOL_TIER_DIAMOND, TOOL_TYPE_PICKAXE)
        ->SetHardness(50.0f)
        ->SetResistance(2000.0f);

    (new BlockTorch(BLOCK_TORCH))
        ->SetSoundDef(woodDef);

    (new BlockCrop(BLOCK_WHEAT_CROPS, 88, 296, 295))
        ->SetSoundDef(grassDef);

    (new BlockFarmland(BLOCK_FARMLAND))
        ->SetSoundDef(gravelDef);

    (new BlockFurnace(BLOCK_FURNACE));
    (new BlockFurnace(BLOCK_BURNING_FURNACE))
        ->SetDrop(BLOCK_FURNACE)
        ->SetLightEmitted(15)
        ->SetOpacity(0);

    (new BlockChest(BLOCK_CHEST))
        ->SetSoundDef(woodDef);

    (new BlockStairs(BLOCK_WOOD_STAIRS, 4))
        ->SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_AXE)
        ->SetSoundDef(woodDef)
        ->SetHardness(2.0f);

    (new BlockStairs(BLOCK_COBBLESTONE_STAIRS, 16))
        ->SetEffectiveTool(TOOL_TIER_WOOD, TOOL_TYPE_PICKAXE)
        ->SetHardness(2.0f);

    (new BlockPumpkin(BLOCK_PUMPKIN))
        ->SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_AXE)
        ->SetSoundDef(woodDef)
        ->SetHardness(1.0f);

    (new BlockPumpkin(BLOCK_JACK_O_LANTERN))
        ->SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_AXE)
        ->SetSoundDef(woodDef)
        ->SetOpacity(0)
        ->SetLightEmitted(15)
        ->SetHardness(1.0f);

    (new BlockCactus(BLOCK_CACTUS))
        ->SetSoundDef(clothDef);

    (new Block(BLOCK_CLAY, 72))
        ->SetDrop(337)
        ->SetDropCount(4)
        ->SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_SHOVEL)
        ->SetHardness(0.6f)
        ->SetSoundDef(gravelDef);

    (new BlockReed(BLOCK_REED))
        ->SetSoundDef(grassDef);

    (new Block(BLOCK_SNOW, 66))
        ->SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_SHOVEL)
        ->SetHardness(0.1f)
        ->SetSoundDef(clothDef)
        ->SetShape(0.0f, 0.0f, 0.0f, 1.0f, 2.0f / 16.0f, 1.0f)
        ->SetOpacity(0)
        ->SetRenderNeighbours(true)
        ->SetRendersToItself(false)
        ->SetIsReplaceable(true)
        ->SetIsReplaceableByPlayer(true)
        ->SetDrop(0);

    (new BlockBookshelf(BLOCK_BOOKSHELF))
        ->SetIsFlammable(true, 30, 20)
        ->SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_AXE)
        ->SetSoundDef(woodDef)
        ->SetHardness(2.0f);

    (new Block(BLOCK_NETHERRACK, 103))
        ->SetEffectiveTool(TOOL_TIER_WOOD, TOOL_TYPE_PICKAXE)
        ->SetHardness(0.4f);

    (new Block(BLOCK_SOUL_SAND, 104))
        ->SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_SHOVEL)
        ->SetSoundDef(gravelDef)
        ->SetHardness(0.5f);

    (new Block(BLOCK_GLOWSTONE, 105))
        ->SetSoundDef(stoneDef)
        ->SetOpacity(0)
        ->SetLightEmitted(15)
        ->SetDrop(348)
        ->SetHardness(0.3f);

    (new BlockNetherPortal(BLOCK_PORTAL));

    (new BlockFire(BLOCK_FIRE))
        ->SetHardness(0.0f);

    (new BlockTNT(BLOCK_TNT))
        ->SetSoundDef(grassDef)
        ->SetHardness(0.0f);

    (new BlockDoor(BLOCK_WOOD_DOOR_BOTTOM, 97, DoorPart::BOTTOM))
        ->SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_AXE)
        ->SetHardness(1.5f)
        ->SetDrop(324)
        ->SetSoundDef(woodDef);

    (new BlockDoor(BLOCK_WOOD_DOOR_TOP, 81, DoorPart::TOP))
        ->SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_AXE)
        ->SetHardness(1.5f)
        ->SetSoundDef(woodDef);

    (new Block(BLOCK_VOID, 0))->SetIsSolid(false)->SetRenderNeighbours(false);
}

Block* Block::GetBlock(uint8_t id)
{
    if (id >= 128)
        return 0;
    return blocks[id];
}

Block::Block(uint8_t id, uint8_t texture)
	: m_Id(id), m_TextureIndex(texture)
{
    blocks[id] = this;
    m_ItemDropId = id;
    SoundDef = blocks[0]->SoundDef;
    m_RequiredTier = TOOL_TIER_NONE;
    m_EffectiveTool = TOOL_TYPE_NONE;
}

BlockRenderType Block::GetRenderType()
{
    return BLOCK_RENDER_TYPE_GENERIC;
}

bool Block::IsFaceRenderable(Chunk* chunk, int x, int y, int z, int face)
{
    if (face == 0 && MinY > 0.0f)
    {
        return true;
    }
    if (face == 1 && MaxY < 1.0f)
    {
        return true;
    }
    if (face == 2 && MinZ > 0.0f)
    {
        return true;
    }
    if (face == 3 && MaxZ < 1.0f)
    {
        return true;
    }
    if (face == 4 && MinX > 0.0f)
    {
        return true;
    }
    if (face == 5 && MaxX < 1.0f)
    {
        return true;
    }
	BlockStorage block = chunk->GetBlockGlobal(x, y, z);
	Block* neighbour = block.GetDef();
    bool valid = (neighbour->GetId() == 0 || neighbour->GetRenderNeighbours());

    if (neighbour->GetId() == GetId() && !m_RenderToItself)
    {
        valid = false;
    }

#ifdef __wii__
    valid = valid && neighbour->GetId() != this->GetId();
#endif
	return valid;
}

void Block::OnPlace(World* world, BaseEntity* entity, int x, int y, int z, int face, int facing)
{
}

void Block::BreakAt(World* world, BaseEntity* entity, int x, int y, int z, bool doDrop)
{
    world->SetBlock(x, y, z, 0);

    int id = GetDropId();
    if (id != 0)
    {
        if (doDrop)
        {
            auto item = std::make_shared<ItemEntity>(world, glm::vec3(x, y, z) + glm::vec3(0.5f), ItemStack{ (uint32_t)id, (int)m_ItemDropCount });
            world->AddEntity(item);
        }
    }
}

void Block::SpawnBreakParticles(World* world, int x, int y, int z)
{
    if (!m_CreatesBreakParticles)
    {
        return;
    }

    if (this->SoundDef)
        world->PlaySound(this->SoundDef->StepSound, glm::vec3(x + 0.5f, y + 0.5f, z + 0.5f), 0.25f);

    for (int i = 0; i < 40; i++)
    {
        TagCompound data;
        data.SetFloat("vx", (Random::GlobalRand.NextFloat() * 2.0f - 1.0f) * 0.1f);
        data.SetFloat("vy", 0.1f + (Random::GlobalRand.NextFloat() * 2.0f - 1.0f) * 0.1f);
        data.SetFloat("vz", (Random::GlobalRand.NextFloat() * 2.0f - 1.0f) * 0.1f);
        data.SetFloat("size", 0.5f);
        data.SetByte("id", GetId());
        data.SetFloat("r", m_TintR);
        data.SetFloat("g", m_TintG);
        data.SetFloat("b", m_TintB);
        auto part = world->SpawnParticle("terrain", x + 0.1f + Random::GlobalRand.NextFloat() * 0.8f, y + 0.1f + Random::GlobalRand.NextFloat() * 0.8f, z + 0.1f + Random::GlobalRand.NextFloat() * 0.8f, &data);
    }
}

void Block::DropAsItemWithChance(World* world, int x, int y, int z)
{

}

void Block::OnBlockExploded(World* world, int x, int y, int z)
{
}

bool Block::UseAt(World* world, BaseEntity* entity, int x, int y, int z)
{
    return false;
}

bool Block::CanExistAt(World* world, int x, int y, int z, int faceId, int facing)
{
    return true;
}

void Block::Tick(World* world, int x, int y, int z)
{

}

void Block::FluidTick(World* world, int x, int y, int z)
{
}

void Block::RandomTick(World* world, int x, int y, int z)
{
    Tick(world, x, y, z);
}

void Block::DisplayTick(World* world, int x, int y, int z)
{
}

AABB Block::GetWorldBoundingBox(World* world, int x, int y, int z)
{
    return AABB(x + MinX, y + MinY, z + MinZ, x + MaxX, y + MaxY, z + MaxZ);
}

AABB Block::GetRaycastBoundingBox(World* world, int x, int y, int z)
{
    if (m_Id == 0)
        return {};
    return GetWorldBoundingBox(world, x, y, z);
}

int Block::GetCollisionBoundingBoxes(World* world, int x, int y, int z, AABB bbs[4])
{
    if (!IsSolid())
        return 0;
    bbs[0] = GetWorldBoundingBox(world, x, y, z);
    return 1;
}

uint8_t Block::GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z)
{
    return m_TextureIndex;
}

Block* Block::SetRenderNeighbours(bool val)
{
    m_RenderNeighbours = val;
    return this;
}

bool Block::GetRenderNeighbours() const
{
    return m_RenderNeighbours;
}

Block* Block::SetColorizeWithBiome(bool val)
{
    m_ColorizeWithBiome = val;
    return this;
}

Block* Block::SetTint(float r, float g, float b)
{
    m_TintR = r;
    m_TintG = g;
    m_TintB = b;
    return this;
}

Block* Block::SetHardness(float hardness)
{
    m_Hardness = hardness;

    if (m_ExplosionResistance < hardness * 5.0f)
    {
        m_ExplosionResistance = hardness * 5.0f;
    }

    return this;
}
float Block::GetHardness() const
{
    return m_Hardness;
}

Block* Block::SetSoundDef(BlockSoundDef* def)
{
    SoundDef = def;
    return this;
}

Block* Block::SetLayer(uint8_t layer)
{
    m_MeshLayer = layer;
    return this;
}

Block* Block::SetOpacity(uint8_t op)
{
    if (op > 15)
        op = 15;
    m_Opacity = op;
    if (op > 1)
        m_IsOpaque = true;
    else
        m_IsOpaque = false;
    return this;
}

Block* Block::SetLightEmitted(uint8_t level)
{
    if (level > 15)
        level = 15;
    m_LightEmitted = level;
    return this;
}

Block* Block::SetShape(float mx, float my, float mz, float xx, float xy, float xz)
{
    MinX = mx;
    MinY = my;
    MinZ = mz;
    MaxX = xx;
    MaxY = xy;
    MaxZ = xz;

    return this;
}

Block* Block::SetMakesSound(bool val)
{
    m_MakesSound = val;
    return this;
}

Block* Block::SetEffectiveTool(ItemToolTier tier, ItemToolType type)
{
    m_RequiredTier = tier;
    m_EffectiveTool = type;
    return this;
}

Block* Block::SetResistance(float resistance)
{
    m_ExplosionResistance = resistance * 3.0f;
    return this;
}

Block* Block::SetDrop(uint32_t id)
{
    m_ItemDropId = id;
    return this;
}

Block* Block::SetDropCount(uint32_t count)
{
    m_ItemDropCount = count;
    return this;
}

Block* Block::SetIconIndex(uint32_t index)
{
    m_IconIndex = index;
    return this;
}
Block* Block::SetIsFlammable(bool val, int ability, int chance)
{
    m_IsFlammable = val;
    m_AbilityToCatchFire = chance;
    m_ChanceToEncourageFire = ability;
    return this;
}

Block* Block::SetIsSolid(bool val)
{
    m_IsSolid = val;
    return this;
}

Block* Block::SetIsOpaque(bool val)
{
    m_IsOpaque = val;
    return this;
}

Block* Block::SetIsSolidToRaycast(bool val)
{
    m_IsSolidToRaycast = val;
    return this;
}

Block* Block::SetIsReplaceable(bool val)
{
    m_IsReplaceable = val;
    return this;
}

Block* Block::SetIsReplaceableByPlayer(bool val)
{
    m_ReplaceableByPlayer = val;
    return this;
}

Block* Block::SetRenderAsIcon(bool val)
{
    m_RenderAsIcon = val;
    return this;
}

Block* Block::SetRendersToItself(bool val)
{
    m_RenderToItself = val;
    return this;
}

Block* Block::SetCreatesBreakParticles(bool val)
{
    m_CreatesBreakParticles = val;
    return this;
}

uint8_t Block::GetRenderLayer() const
{
    return m_MeshLayer;
}

uint8_t Block::GetId() const
{
    if (m_Id == BLOCK_VOID) return BLOCK_AIR;
    return m_Id;
}

uint8_t Block::GetOpacity() const
{
    return m_Opacity;
}

uint8_t Block::GetLightEmitted() const
{
    return m_LightEmitted;
}

uint8_t Block::GetIconIndex() const
{
    return m_IconIndex;
}

bool Block::IsFlammable() const
{
    return m_IsFlammable;
}

bool Block::IsSolid() const
{
    return m_IsSolid;
}

bool Block::IsOpaque() const
{
    return m_IsOpaque;
}

bool Block::IsSolidToRaycast() const
{
    return m_IsSolidToRaycast;
}

bool Block::IsReplaceable() const
{
    return m_IsReplaceable;
}

bool Block::IsReplaceableByPlayer() const
{
    return m_ReplaceableByPlayer;
}

bool Block::BlocksLightToTheSides() const
{
    return m_BlocksSideLight;
}

bool Block::MakesSound() const
{
    return m_MakesSound;
}

bool Block::RendersAsIcon() const
{
    return m_RenderAsIcon;
}

bool Block::IsFullyOpaqueBlock() const
{
    return IsSolid() && GetOpacity() == 15 && MinX == 0.0f && MaxX == 1.0f && MinZ == 0.0f && MaxZ == 1.0f && MinY == 0.0f && MaxY == 1.0f;
}

float Block::GetBlockResistance(BaseEntity* entity) const
{
    return m_ExplosionResistance / 5.0f;
}

int Block::GetAbilityToCatchFire() const
{
    return m_AbilityToCatchFire;
}

int Block::GetChanceOfFire() const
{
    return m_ChanceToEncourageFire;
}

ItemToolTier Block::GetRequiredTier() const
{
    return m_RequiredTier;
}

ItemToolType Block::GetEffectiveToolType() const
{
    return m_EffectiveTool;
}

int Block::GetDropId()
{
    return m_ItemDropId;
}

int Block::GetDropCount()
{
    return m_ItemDropCount;
}

Block* Block::SetTexture(uint8_t x, uint8_t y)
{
    m_TextureIndex = x + y * 16;
    return this;
}

float Block::CalculateLightLevelForFace(Chunk* chunk, int x, int y, int z, int face)
{
    if (!chunk)
    {
        return 1.0f;
    }

    int light = 0;

    switch (face)
    {
    case 0:
        light = chunk->ChunkWorld->GetMixedLight(x, (int)floor(y - 0.1f + MinY), z);
        break;
    case 1:
        light = chunk->ChunkWorld->GetMixedLight(x, (int)floor(y + 0.1f + MaxY), z);
        break;
    case 2:
        light = chunk->ChunkWorld->GetMixedLight(x, y, (int)floor(z - 0.1f + MinZ));
        break;
    case 3:
        light = chunk->ChunkWorld->GetMixedLight(x, y, (int)floor(z + 0.1f + MaxZ));
        break;
    case 4:
        light = chunk->ChunkWorld->GetMixedLight((int)floor(x - 0.1f + MinX), y, z);
        break;
    case 5:
        light = chunk->ChunkWorld->GetMixedLight((int)floor(x + 0.1f + MaxX), y, z);
        break;
    default:
        light = chunk->ChunkWorld->GetMixedLight(x, y, z);
        break;
    }

    return chunk->ChunkWorld->LightLUT[light];
}