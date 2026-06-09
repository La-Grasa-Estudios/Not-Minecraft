#pragma once

#include "common/Random.h"

#include <vector>

class SimplexNoise
{
public:
	SimplexNoise() = default;
	SimplexNoise(Random& random);
	void Sample(float* arr, float x, float y, int sizeX, int sizeY, float scaleX, float scaleY, float amplitude);
	float Sample(float x, float y, float scaleX, float scaleY);
private:
	int permutations[512];
	float xOrigin;
	float yOrigin;
	float zOrigin;
};

class SimplexNoiseOctaves
{
private:
	std::vector<SimplexNoise> generatorCollection;
	int m_Octaves;
public:

	SimplexNoiseOctaves() = default;
	SimplexNoiseOctaves(Random& random, int octaves);

	float Sample(int localX, int localZ, float scaleX, float scaleZ, float lacunarity, float persistence);
	float* SampleBiome2D(float* noiseArray, int localX, int localZ, int sizeX, int sizeZ, float scaleX, float scaleZ, float lacunarity, float persistence);
};