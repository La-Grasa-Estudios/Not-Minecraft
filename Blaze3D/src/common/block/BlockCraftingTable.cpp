#include "BlockCraftingTable.h"

#include "client/Client.h"
#include "client/ui/ScreenCrafting.h"

BlockCraftingTable::BlockCraftingTable(uint8_t id) : Block(id, 0)
{
	SetTexture(11, 3);
	SetEffectiveTool(TOOL_TIER_NONE, TOOL_TYPE_AXE);
	SetHardness(2.0f);
}

uint8_t BlockCraftingTable::GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z)
{
	if (face == 0)
	{
		return 4;
	}
	if (face == 1)
	{
		return (11 + 2 * 16);
	}
	if (face == 2)
	{
		return (12 + 3 * 16);
	}
	return m_TextureIndex;
}

bool BlockCraftingTable::UseAt(World* world, BaseEntity* entity, int x, int y, int z)
{
	auto& client = Client::GetInstance();
	client.SetScreen(std::make_shared<ScreenCrafting>(client.GraphicsDevice));
	return true;
}
