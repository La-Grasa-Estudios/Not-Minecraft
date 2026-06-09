#include "BlockSlab.h"

#include "common/Chunk.h"

BlockSlab::BlockSlab(uint8_t id, uint8_t textureTop, uint8_t textureSide) : Block(id, textureSide)
{
	m_TopTexture = textureTop;
	m_TextureIndex = textureSide;
	SetRenderNeighbours(true);
	SetShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	SetEffectiveTool(TOOL_TIER_WOOD, TOOL_TYPE_PICKAXE);
	SetHardness(2.0f);
	SetOpacity(0);
	SetIsOpaque(true);
	m_BlocksSideLight = false;
}

uint8_t BlockSlab::GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z)
{
	return face <= 1 ? m_TopTexture : m_TextureIndex;
}

bool BlockSlab::IsFaceRenderable(Chunk* chunk, int x, int y, int z, int face)
{
	return face == 1 || Block::IsFaceRenderable(chunk, x, y, z, face);
}
