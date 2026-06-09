#include "BlockTorch.h"

#include "common/World.h"
#include "common/Chunk.h"
#include <thirdparty/glm/ext.hpp>

constexpr int FACING_UP = 1;
constexpr int FACING_NORTH = 2;
constexpr int FACING_SOUTH = 3;
constexpr int FACING_WEST = 4;
constexpr int FACING_EAST = 5;

BlockTorch::BlockTorch(uint8_t id) : Block(id, 80)
{
	SetIsSolid(false);
	SetIsReplaceable(true);
	SetRenderNeighbours(true);
	SetOpacity(0);
	SetRenderAsIcon(true);
	SetIconIndex(80);
	SetHardness(0.0f);
	SetLightEmitted(15);
}

void BlockTorch::OnPlace(World* world, BaseEntity* entity, int x, int y, int z, int face, int facing)
{
	auto existing = world->GetBlock(x, y, z);
	existing.Metadata = face;
	if (face == 0)
	{
		BreakAt(world, NULL, x, y, z, true);
	}
	else
	{
		world->SetBlock(x, y, z, existing);
	}
}

void BlockTorch::Tick(World* world, int x, int y, int z)
{
    if (!CanExistAt(world, x, y, z, world->GetBlock(x, y, z).Metadata, 0))
    {
        BreakAt(world, NULL, x, y, z, true);
    }
}

void BlockTorch::DisplayTick(World* world, int x, int y, int z)
{
    auto at = world->GetBlock(x, y, z);

    if (at.Metadata == FACING_NORTH)
    {
        world->SpawnParticle("fire", x + 0.5f, y + 0.75f, z + 0.5f + 0.25f);
        return;
    }
    if (at.Metadata == FACING_SOUTH)
    {
        world->SpawnParticle("fire", x + 0.5f, y + 0.75f, z + 0.25f);
        return;
    }
    if (at.Metadata == FACING_WEST)
    {
        world->SpawnParticle("fire", x + 0.5f + 0.25f, y + 0.75f, z + 0.5f);
        return;
    }
    if (at.Metadata == FACING_EAST)
    {
        world->SpawnParticle("fire", x + 0.25f, y + 0.75f, z + 0.5f);
        return;
    }

    world->SpawnParticle("fire", x + 0.5f, y + 0.65f, z + 0.5f);
}

bool BlockTorch::CanExistAt(World* world, int x, int y, int z, int faceId, int facing)
{
    if (faceId == 1)
    {
        auto block = world->GetBlock(x, y - 1, z).GetDef();
        if (block->MaxY <= 0.999f || !block->IsSolid())
        {
            return false;
        }
    }
    else if (faceId == 2)
    {
        auto block = world->GetBlock(x, y, z + 1).GetDef();
        if (block->MaxZ <= 0.999f || !block->IsSolid())
            return false;
    }
    else if (faceId == 3)
    {
        auto block = world->GetBlock(x, y, z - 1).GetDef();
        if (block->MinZ >= 0.001f || !block->IsSolid())
            return false;
    }
    else if (faceId == 4)
    {
        auto block = world->GetBlock(x + 1, y, z).GetDef();
        if (block->MaxX <= 0.999f || !block->IsSolid())
            return false;
    }
    else if (faceId == 5)
    {
        auto block = world->GetBlock(x - 1, y, z).GetDef();
        if (block->MinX >= 0.001f || !block->IsSolid())
            return false;
    }

	return faceId > 0;
}

AABB BlockTorch::GetRaycastBoundingBox(World* world, int x, int y, int z)
{
    auto data = world->GetBlock(x, y, z);

    const float corner1 = 5.0f / 16.0f;
    const float corner2 = 11.0f / 16.0f;

    if (data.Metadata == FACING_NORTH)
    {
        AABB bb(corner1, 0.1f, corner1, corner2, 12.0f / 16.0f, corner2);
        bb.move(x, y, z + 6.0f / 16.0f);
        return bb;
    } else if (data.Metadata == FACING_SOUTH)
    {
        AABB bb(corner1, 0.1f, corner1, corner2, 12.0f / 16.0f, corner2);
        bb.move(x, y, z - 6.0f / 16.0f);
        return bb;
    } else if (data.Metadata == FACING_WEST)
    {
        AABB bb(corner1, 0.1f, corner1, corner2, 12.0f / 16.0f, corner2);
        bb.move(x + 6.0f / 16.0f, y, z);
        return bb;
    }
    else if (data.Metadata == FACING_EAST)
    {
        AABB bb(corner1, 0.1f, corner1, corner2, 12.0f / 16.0f, corner2);
        bb.move(x - 6.0f / 16.0f, y, z);
        return bb;
    }
    else
    {
        AABB bb(corner1, 0.0f, corner1, corner2, 11.0f / 16.0f, corner2);
        bb.move(x, y, z);
        return bb;
    }

    return Block::GetRaycastBoundingBox(world, x, y, z);
}

BlockRenderType BlockTorch::GetRenderType()
{
    return BLOCK_RENDER_TYPE_TORCH;
}
