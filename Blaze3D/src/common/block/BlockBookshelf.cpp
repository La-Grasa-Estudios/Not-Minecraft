#include "BlockBookshelf.h"

BlockBookshelf::BlockBookshelf(uint8_t id) : Block(id, 0)
{
}

uint8_t BlockBookshelf::GetTextureForFace(uint8_t face, Chunk* chunk, int x, int y, int z)
{
	return face <= 1 ? 4 : 35;
}
