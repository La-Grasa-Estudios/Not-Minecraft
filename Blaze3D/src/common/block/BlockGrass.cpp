#include "BlockGrass.h"

#include <common/Random.h>

#include <common/Chunk.h>
#include <common/World.h>

BlockGrass::BlockGrass(uint8_t id)
    : Block(id, 3)
{
    SetDrop(BLOCK_DIRT);
}

void BlockGrass::RandomTick(World* world, int x, int y, int z)
{
    if (world->GetMixedLightNoDecrease(x, y + 1, z) < 4)
    {
        world->SetBlock(x, y, z, BLOCK_DIRT);
        return;
    }

    for (int i = 0; i < 4; ++i) {
        int xt = x + Random::GlobalRand.NextInt(3) - 1;
        int yt = y + Random::GlobalRand.NextInt(5) - 3;
        int zt = z + Random::GlobalRand.NextInt(3) - 1;
        auto block = world->GetBlock(xt, yt, zt);
        if (block.Id == BLOCK_DIRT) {
            if (world->GetMixedLight(xt, yt + 1, zt) >= 9)
                world->SetBlock(xt, yt, zt, BLOCK_GRASS);
        }
    }
}

uint8_t BlockGrass::GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z)
{
    if (face == 1)
    {
        return 0;
    }

    if (chunk)
    {
        bool snowy = chunk->GetBlockGlobal(x, y + 1, z).Id == BLOCK_SNOW;
        return face < 2 ? m_TextureIndex : (snowy ? 68 : m_TextureIndex);
    }
    return m_TextureIndex;
}

BlockRenderType BlockGrass::GetRenderType()
{
    return BLOCK_RENDER_TYPE_GRASS_BLOCK;
}
