#pragma once

class Biome
{
public:
	Biome(int topBlock, int fillerBlock, int trees, int cacti, bool snowy);
	int TreesPerChunk = 0;
	int CactiPerChunk = 0;
	int TopBlock = 0;
	int FillerBlock = 0;
	bool Snowy = false;
	const char* Name;
};