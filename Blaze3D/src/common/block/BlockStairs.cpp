#include "BlockStairs.h"

#include "common/World.h"

BlockStairs::BlockStairs(uint8_t id, uint8_t texture) : Block(id, texture)
{
	SetRenderNeighbours(true);
	SetOpacity(0);
	SetIsOpaque(true);
	m_BlocksSideLight = false;
}

void BlockStairs::OnPlace(World* world, BaseEntity* entity, int x, int y, int z, int face, int facing)
{
	auto block = world->GetBlock(x, y, z);
	block.Metadata = facing;
	world->SetBlockNoNotify(x, y, z, block);
}

int BlockStairs::GetCollisionBoundingBoxes(World* world, int x, int y, int z, AABB bbs[4])
{
	SetShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	Block::GetCollisionBoundingBoxes(world, x, y, z, bbs);

	auto block = world->GetBlock(x, y, z);
	if (block.Metadata == 0)
	{
		SetShape(0.0f, 0.5f, 0.0f, 1.0f, 1.0f, 0.5f);
	}
	if (block.Metadata == 1)
	{
		SetShape(0.0f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f);
	}
	if (block.Metadata == 2)
	{
		SetShape(0.0f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f);
	}
	if (block.Metadata == 3)
	{
		SetShape(0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f);
	}
	bbs[1] = Block::GetWorldBoundingBox(world, x, y, z);
	SetShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	return 2;
}

bool BlockStairs::IsFaceRenderable(Chunk* chunk, int x, int y, int z, int face)
{
	BlockStorage block = chunk->GetBlockGlobal(x, y, z);
	Block* neighbour = block.GetDef();
	bool valid = (neighbour->GetId() == 0 || neighbour->GetRenderNeighbours());

	if (face == 0 && MinY > 0.0f)
	{
		return false;
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

	if (neighbour->GetId() == GetId())
	{
		return false;
	}

	return valid;
}

BlockRenderType BlockStairs::GetRenderType()
{
	return BLOCK_RENDER_TYPE_STAIRS;
}
