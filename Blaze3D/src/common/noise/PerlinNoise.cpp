#include "PerlinNoise.h"

#include <cmath>
#include <common/Random.h>

#include <thirdparty/glm/common.hpp>

inline float lerp2(float u, float v,
    float a00, float a01,
    float a10, float a11) {
    return glm::mix(v,
        glm::mix(u, a00, a01),
        glm::mix(u, a10, a11));
}

inline float lerp3(float u, float v, float w,
    float a000, float a001,
    float a010, float a011,
    float a100, float a101,
    float a110, float a111) {
    return glm::mix(w,
        lerp2(u, v, a000, a001, a010, a011),
        lerp2(u, v, a100, a101, a110, a111));
}

PerlinNoise::PerlinNoise(Random& random, bool useOffset)
{
    offsetX = offsetY = offsetZ = 0;

    if (useOffset) {
        offsetX = random.NextFloat() * 256.0f;
        offsetY = random.NextFloat() * 256.0f;
        offsetZ = random.NextFloat() * 256.0f;
    }

    for (int i = 0; i < 256; i++) {
        permutations[i] = i;
    }

    for (int i = 0; i < 256; i++) {
        int j = random.NextInt(256 - i) + i;
        int k = permutations[i];

        permutations[i] = permutations[j];
        permutations[j] = k;

        // Repeat first 256 values to avoid buffer overflow
        permutations[i + 256] = permutations[i];
    }
}

float PerlinNoise::Sample(float x, float y)
{
	return Sample(x, y, 0.0f);
}

float PerlinNoise::Sample(float x, float y, float z)
{
    x += offsetX;
    y += offsetY;
    z += offsetZ;

    int floorX = floor(x);
    int floorY = floor(y);
    int floorZ = floor(z);

    // Find unit cube that contains point.
    int X = floorX & 0xFF;
    int Y = floorY & 0xFF;
    int Z = floorZ & 0xFF;

    // Find local x, y, z of point in cube.
    x -= floorX;
    y -= floorY;
    z -= floorZ;

    // Compute fade curves for x, y, z.
    float u = fade(x);
    float v = fade(y);
    float w = fade(z);

    // Hash coordinates of the 8 cube corners.
    int A = permutations[X] + Y;
    int AA = permutations[A] + Z;
    int AB = permutations[A + 1] + Z;
    int B = permutations[X + 1] + Y;
    int BA = permutations[B] + Z;
    int BB = permutations[B + 1] + Z;

    return lerp(
        w,
        lerp(
            v,
            lerp(
                u,
                grad(permutations[AA], x, y, z),
                grad(permutations[BA], x - 1.0, y, z)),
            lerp(
                u,
                grad(permutations[AB], x, y - 1.0, z),
                grad(permutations[BB], x - 1.0, y - 1.0, z))),
        lerp(
            v,
            lerp(
                u,
                grad(permutations[AA + 1], x, y, z - 1.0),
                grad(permutations[BA + 1], x - 1.0, y, z - 1.0)),
            lerp(
                u,
                grad(permutations[AB + 1], x, y - 1.0, z - 1.0),
                grad(permutations[BB + 1], x - 1.0, y - 1.0, z - 1.0)))
    );
}

float PerlinNoise::SampleOctave(float x, float y, float z, float scale, int octaves)
{
    float n1 = .0f;

    float freq = 1.0f;

    for (int i = 0; i < 8; i++)
    {
        int octave = i + 1;
        float sc = 1.0f / freq;
        float n = Sample(x * scale * sc, y * scale * sc, z * scale * sc);
        n1 += n * freq;
        freq /= 2;
    }

    return n1;
}

float PerlinNoise::SampleXYZ(float x, float y, float z, float yScale, float yMax)
{
    x += offsetX;
    y += offsetY;
    z += offsetZ;

    int floorX = floor(x);
    int floorY = floor(y);
    int floorZ = floor(z);

    x -= floorX;
    y -= floorY;
    z -= floorZ;

    float yOffset = 0.0;
    if (yScale != 0.0) {
        if (yMax >= 0.0 && yMax < y) {
            yOffset = yMax;
        }
        else {
            yOffset = y;
        }

        yOffset = floor(yOffset / yScale + 1.0000000116860974E-7f) * yScale;
    }
    else {
        yOffset = 0.0;
    }

    return SampleXYZ(floorX, floorY, floorZ, x, y - yOffset, z, y);
}

float PerlinNoise::SampleXYZ(int floorX, int floorY, int floorZ, float localX, float localOffsetY, float localZ, float localY)
{
    int X = floorX & 0xFF;
    int Y = floorY & 0xFF;
    int Z = floorZ & 0xFF;

    int A = permutations[X] + Y;
    int AA = permutations[A] + Z;
    int AB = permutations[A + 1] + Z;
    int B = permutations[X + 1] + Y;
    int BA = permutations[B] + Z;
    int BB = permutations[B + 1] + Z;

    // Calculate dot of hashed gradient vector against 8 location vectors.
    float grad0 = grad(permutations[AA], localX, localOffsetY, localZ);
    float grad1 = grad(permutations[BA], localX - 1.0, localOffsetY, localZ);

    float grad2 = grad(permutations[AB], localX, localOffsetY - 1.0, localZ);
    float grad3 = grad(permutations[BB], localX - 1.0, localOffsetY - 1.0, localZ);

    float grad4 = grad(permutations[AA + 1], localX, localOffsetY, localZ - 1.0);
    float grad5 = grad(permutations[BA + 1], localX - 1.0, localOffsetY, localZ - 1.0);

    float grad6 = grad(permutations[AB + 1], localX, localOffsetY - 1.0, localZ - 1.0);
    float grad7 = grad(permutations[BB + 1], localX - 1.0, localOffsetY - 1.0, localZ - 1.0);

    float u = fade(localX);
    float v = fade(localY);
    float w = fade(localZ);

    return lerp3(u, v, w, grad0, grad1, grad2, grad3, grad4, grad5, grad6, grad7);
}

void PerlinNoise::SampleAlpha(float arr[], float x, float y, float z, int sizeX, int sizeY, int sizeZ, float scaleX, float scaleY, float scaleZ, float frequency)
{
    int ndx = 0;
    frequency = 1.0f / frequency;
    int flagY = -1;

    float lerp0 = 0.0f;
    float lerp1 = 0.0f;
    float lerp2 = 0.0f;
    float lerp3 = 0.0f;

    // Iterate over a collection of noise points
    for (int sX = 0; sX < sizeX; sX++) {
        for (int sZ = 0; sZ < sizeZ; sZ++) {
            for (int sY = 0; sY < sizeY; sY++) {
                float curX = (x + (float)sX) * scaleX + offsetX;
                float curY = (y + (float)sY) * scaleY + offsetY;
                float curZ = (z + (float)sZ) * scaleZ + offsetZ;

                int floorX = floor(curX);
                int floorY = floor(curY);
                int floorZ = floor(curZ);

                // Find unit cube that contains point.
                int X = floorX & 0xFF;
                int Y = floorY & 0xFF;
                int Z = floorZ & 0xFF;

                // Find local x, y, z of point in cube.
                curX -= floorX;
                curY -= floorY;
                curZ -= floorZ;

                // Compute fade curves for x, y, z.
                float u = fade(curX);
                float v = fade(curY);
                float w = fade(curZ);

                if (sY == 0 || Y != flagY) {
                    flagY = Y;

                    int A = permutations[X] + Y;
                    int AA = permutations[A] + Z;
                    int AB = permutations[A + 1] + Z;
                    int B = permutations[X + 1] + Y;
                    int BA = permutations[B] + Z;
                    int BB = permutations[B + 1] + Z;

                    lerp0 = lerp(
                        u,
                        grad(permutations[AA], curX, curY, curZ),
                        grad(permutations[BA], curX - 1.0f, curY, curZ)
                    );

                    lerp1 = lerp(
                        u,
                        grad(permutations[AB], curX, curY - 1.0f, curZ),
                        grad(permutations[BB], curX - 1.0f, curY - 1.0f, curZ)
                    );

                    lerp2 = lerp(
                        u,
                        grad(permutations[AA + 1], curX, curY, curZ - 1.0f),
                        grad(permutations[BA + 1], curX - 1.0f, curY, curZ - 1.0f)
                    );

                    lerp3 = lerp(
                        u,
                        grad(permutations[AB + 1], curX, curY - 1.0f, curZ - 1.0f),
                        grad(permutations[BB + 1], curX - 1.0f, curY - 1.0f, curZ - 1.0f)
                    );
                }

                float res = lerp(w, lerp(v, lerp0, lerp1), lerp(v, lerp2, lerp3));

                arr[ndx++] += res * frequency;
            }
        }
    }
}

float PerlinNoise::lerp(float delta, float start, float end)
{
    return start + delta * (end - start);
}

float PerlinNoise::grad(int hash, float x, float y, float z)
{
    switch (hash & 0xF) {
    case 0x0:
        return x + y;
    case 0x1:
        return -x + y;
    case 0x2:
        return x - y;
    case 0x3:
        return -x - y;
    case 0x4:
        return x + z;
    case 0x5:
        return -x + z;
    case 0x6:
        return x - z;
    case 0x7:
        return -x - z;
    case 0x8:
        return y + z;
    case 0x9:
        return -y + z;
    case 0xA:
        return y - z;
    case 0xB:
        return -y - z;
    case 0xC:
        return y + x;
    case 0xD:
        return -y + z;
    case 0xE:
        return y - x;
    case 0xF:
        return -y - z;
    default:
        return 0; // never happens
    }
}

float PerlinNoise::fade(float t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

PerlinNoiseOctaves::PerlinNoiseOctaves(Random& var1, int var2)
{
    m_Octaves = var2;
    generatorCollection.resize(var2);

    for (int var3 = 0; var3 < var2; ++var3) {
        generatorCollection[var3] = PerlinNoise(var1, true);
    }
}

float PerlinNoiseOctaves::SampleOctaves(float localX, float localZ)
{
    float accum = 0.0f;
    float frequency = 1.0f;

    for (int i = 0; i < m_Octaves; ++i) {
        accum += generatorCollection[i].Sample(localX * frequency, localZ * frequency) / frequency;
        frequency /= 2.0f;
    }

    return accum;
}

float* PerlinNoiseOctaves::Sample(float* noiseArray, float localX, float localY, float localZ, int sizeX, int sizeY, int sizeZ, float scaleX, float scaleY, float scaleZ)
{
    if (noiseArray == NULL) {
        noiseArray = new float[sizeX * sizeY * sizeZ];
    }
    else {
        auto size = sizeX * sizeY * sizeZ;
        for (int i = 0; i < size; ++i) {
            noiseArray[i] = 0.0f;
        }
    }

    float frequency = 1.0f;

    for (int i = 0; i < m_Octaves; ++i) {
        generatorCollection[i].SampleAlpha(noiseArray, localX, localY, localZ, sizeX, sizeY, sizeZ, scaleX * frequency, scaleY * frequency, scaleZ * frequency, frequency);
        frequency /= 2.0f;
    }

    return noiseArray;
}

float* PerlinNoiseOctaves::Sample2D(float* noiseArray, int localX, int localZ, int sizeX, int sizeZ, float scaleX, float scaleZ)
{
    return Sample(noiseArray, (float)localX, 10.0f, (float)localZ, sizeX, 1, sizeZ, scaleX, 1.0f, scaleZ);
}
