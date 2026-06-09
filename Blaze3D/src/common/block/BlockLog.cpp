#include "BlockLog.h"

BlockLog::BlockLog(uint8_t id)
    : Block(id, 20)
{
}

uint8_t BlockLog::GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z)
{
    return face <= 1 ? 21 : m_TextureIndex;
}