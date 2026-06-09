#include "BlockCrossShaped.h"

#include "common/World.h"

BlockCrossShaped::BlockCrossShaped(uint8_t id) : Block(id, 0)
{
	SetIsSolid(false);
	SetIsReplaceable(true);
	SetRenderNeighbours(true);
	SetOpacity(0);
	const float offset = 1.0f / 16.0f;
	SetShape(offset, 0.0f, offset, 1 - offset, 1 - offset, 1 - offset);
	SetRenderAsIcon(true);
	SetIconIndex(15);
	SetHardness(0.0f);
}

bool BlockCrossShaped::CanExistAt(World* world, int x, int y, int z, int faceId, int facing)
{
	return world->GetBlock(x, y - 1, z).GetDef()->IsSolid();
}

BlockRenderType BlockCrossShaped::GetRenderType()
{
	return BLOCK_RENDER_TYPE_CROSS;
}
