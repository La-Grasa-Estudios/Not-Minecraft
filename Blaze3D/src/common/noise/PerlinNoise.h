#pragma once

#include "common/Random.h"

#include <vector>

class PerlinNoise
{
public:
	PerlinNoise() = default;
	PerlinNoise(Random& random, bool useOffset);
	float Sample(float x, float y);
	float Sample(float x, float y, float z);
    float SampleOctave(float x, float y, float z, float scale, int octaves);
    float SampleXYZ(float x, float y, float z, float yScale, float yMax);
    float SampleXYZ(int floorX, int floorY, int floorZ, float localX, float localOffsetY, float localZ, float localY);
    void SampleAlpha(
        float arr[],
        float x, float y, float z,
        int sizeX, int sizeY, int sizeZ,
        float scaleX, float scaleY, float scaleZ,
        float frequency
    );
private:
    float lerp(float delta, float start, float end);
    float grad(int hash, float x, float y, float z);
    float fade(float t);
    int permutations[512];
    float offsetX, offsetY, offsetZ;
};

class PerlinNoiseOctaves
{
private:
	std::vector<PerlinNoise> generatorCollection;
	int m_Octaves;
public:

    PerlinNoiseOctaves() = default;
    PerlinNoiseOctaves(Random& random, int octaves);

	float SampleOctaves(float localX, float localZ);

	float* Sample(float* noiseArray, float localX, float localY, float localZ, int sizeX, int sizeY, int sizeZ, float scaleX, float scaleY, float scaleZ);

	float* Sample2D(float* noiseArray, int localX, int localZ, int sizeX, int sizeZ, float scaleX, float scaleZ);
};