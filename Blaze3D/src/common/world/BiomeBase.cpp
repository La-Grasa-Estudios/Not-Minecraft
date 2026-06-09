#include "BiomeBase.h"

Biome::Biome(int topBlock, int fillerBlock, int trees, int cacti, bool snowy) :
	TreesPerChunk(trees),
	TopBlock(topBlock),
	FillerBlock(fillerBlock),
	CactiPerChunk(cacti), 
	Snowy(snowy)
{
}
