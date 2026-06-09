#include "BlockNetherPortal.h"

#include "common/PortalStructure.h"

BlockNetherPortal::BlockNetherPortal(uint8_t id) : Block(id, 0)
{
	SetIsSolid(false);
	SetRenderNeighbours(true);
	SetRendersToItself(false);
	SetLayer(1);
	SetHardness(1232130.0f);
	SetLightEmitted(12);
	SetOpacity(0);
	SetDrop(0);
	SetTexture(14, 0);
	SetResistance(0.0f);
}

bool BlockNetherPortal::IsFaceRenderable(Chunk* chunk, int x, int y, int z, int face)
{
	if (face < 2)
		return false;
	return Block::IsFaceRenderable(chunk, x, y, z, face);
}

void BlockNetherPortal::Tick(World* world, int x, int y, int z)
{
	int ox, oy, oz;
	int dir = PortalStructure::GetPortalDirection(world, x, y, z, ox, oy, oz);
	if (dir == -1)
	{
		BreakAt(world, NULL, x, y, z, false);
		world->NotifyNeighboursWithDelay(x, y, z, 0, TICK_TYPE_NORMAL);
		return;
	}
	world->SetBlockNoNotify(x, y, z, { GetId(), (uint8_t)dir });
}

AABB BlockNetherPortal::GetWorldBoundingBox(World* world, int x, int y, int z)
{
	auto dir = world->GetBlock(x, y, z).Metadata;
	if (dir == 1)
	{
		SetShape(0.3f, 0.0f, 0.0f, 0.6f, 1.0f, 1.0f);
	}
	else
	{
		SetShape(0.0f, 0.0f, 0.3f, 1.0f, 1.0f, 0.6f);
	}
	return Block::GetWorldBoundingBox(world, x, y, z);
}

BlockRenderType BlockNetherPortal::GetRenderType()
{
	return BLOCK_RENDER_TYPE_PORTAL;
}
