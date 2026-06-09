#include "BlockCrop.h"

#include "common/Random.h"

#include "common/World.h"
#include "common/entity/ItemEntity.h"

BlockCrop::BlockCrop(uint8_t id, uint8_t texture, uint32_t drop, uint32_t seedDrop) : Block(id, texture)
{
	m_CropDrop = drop;
	m_SeedDrop = seedDrop;
	SetDrop(0);
	SetOpacity(0);
	SetIsSolid(0);
	SetRenderNeighbours(true);
	SetRendersToItself(true);
	SetHardness(0.0f);
}

void BlockCrop::BreakAt(World* world, BaseEntity* entity, int x, int y, int z, bool doDrop)
{
	if (doDrop)
	{
		auto block = world->GetBlock(x, y, z);
		if (block.Metadata == 7)
		{
			auto item = std::make_shared<ItemEntity>(world, glm::vec3(x, y, z) + glm::vec3(0.5f), ItemStack{ m_CropDrop, 1 + Random::GlobalRand.NextInt(3) });
			world->AddEntity(item);
			item = std::make_shared<ItemEntity>(world, glm::vec3(x, y, z) + glm::vec3(0.5f), ItemStack{ m_SeedDrop, 1 + Random::GlobalRand.NextInt(3) });
			world->AddEntity(item);
		}
		else
		{
			auto item = std::make_shared<ItemEntity>(world, glm::vec3(x, y, z) + glm::vec3(0.5f), ItemStack{ m_SeedDrop, 1 });
			world->AddEntity(item);
		}
	}
	Block::BreakAt(world, entity, x, y, z, doDrop);
}

void BlockCrop::RandomTick(World* world, int x, int y, int z)
{
    if (Random::GlobalRand.NextInt(30) <= 10)
    {
        auto block = world->GetBlock(x, y, z);

        if (block.Metadata < 7)
        {
            block.Metadata++;
            world->SetBlock(x, y, z, block);
        }
    }
	
    Tick(world, x, y, z);
}

void BlockCrop::Tick(World* world, int x, int y, int z)
{
	if (!CanExistAt(world, x, y, z, 0, 0))
	{
		BreakAt(world, NULL, x, y, z, true);
	}
}

bool BlockCrop::CanExistAt(World* world, int x, int y, int z, int faceId, int facing)
{
	return world->GetBlock(x, y - 1, z).Id == BLOCK_FARMLAND && world->GetMixedLightNoDecrease(x, y, z) >= 9;
}

uint8_t BlockCrop::GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z)
{
	if (chunk)
	{
		return m_TextureIndex + chunk->GetBlockGlobal(x, y, z).Metadata;
	}
	return m_TextureIndex + 3;
}

bool BlockCrop::IsFaceRenderable(Chunk* chunk, int x, int y, int z, int face)
{
	return face > 1;
}

AABB BlockCrop::GetRaycastBoundingBox(World* world, int x, int y, int z)
{
    const float offset = 2.0f / 16.0f;
    float size = ((float)world->GetBlock(x, y, z).Metadata + 1.0f) / 8.0f;
    size = 0.8f * size;
    AABB bb = AABB(offset, -0.1f, offset, 1 - offset, size, 1 - offset);
    bb.move(x, y, z);
    return bb;
}

BlockRenderType BlockCrop::GetRenderType()
{
    return BLOCK_RENDER_TYPE_CROP;
}
