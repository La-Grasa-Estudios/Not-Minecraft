#include "SimplexNoise.h"

#include <thirdparty/glm/ext.hpp>

static int fastFloor(float float1) {
    return (float1 > 0.0) ? ((int)float1) : ((int)float1 - 1);
}

static float dot(const int* arr, float float2, float float4) {
    return arr[0] * float2 + arr[1] * float4;
}

static const float UNSKEW_FACTOR_2D = (3.0f - glm::sqrt(3.0f)) / 6.0f;
static const float SKEW_FACTOR_2D = 0.5f * (glm::sqrt(3.0f) - 1.0f);

static const int gradients[] = {
     1,  1,  0,
    -1,  1,  0,
     1, -1,  0,
    -1, -1,  0,
     1,  0,  1,
    -1,  0,  1,
     1,  0, -1,
    -1,  0, -1,
     0,  1,  1,
     0, -1,  1,
     0,  1, -1,
     0, -1, -1
};

SimplexNoise::SimplexNoise(Random& random)
{
    xOrigin = random.NextFloat() * 256.0;
    yOrigin = random.NextFloat() * 256.0;
    zOrigin = random.NextFloat() * 256.0;

    for (int i = 0; i < 256; ++i) {
        permutations[i] = i;
    }

    for (int i = 0; i < 256; ++i) {
        int permNdx = random.NextInt(256 - i) + i;
        int perm = permutations[i];

        permutations[i] = permutations[permNdx];
        permutations[permNdx] = perm;
        permutations[i + 256] = permutations[i];
    }
}

void SimplexNoise::Sample(float* arr, float x, float y, int sizeX, int sizeY, float scaleX, float scaleY, float amplitude)
{
    int ndx = 0;
    
    for (int sX = 0; sX < sizeX; ++sX) {
        float curX = (x + sX) * scaleX + xOrigin;
        
        for (int sY = 0; sY < sizeY; ++sY) {
            float curY = (y + sY) * scaleY + yOrigin;
            
            float s = (curX + curY) * SKEW_FACTOR_2D;
            int i = fastFloor(curX + s);
            int j = fastFloor(curY + s);
            
            float t = (i + j) * UNSKEW_FACTOR_2D;
            float x0 = i - t;
            float y0 = j - t;
            float xDist = curX - x0;
            float yDist = curY - y0;
            
            int offsetI;
            int offsetJ;
            if (xDist > yDist) {
                offsetI = 1;
                offsetJ = 0;
            }
            else {
                offsetI = 0;
                offsetJ = 1;
            }
            
            float offsetMidX = xDist - offsetI + UNSKEW_FACTOR_2D;
            float offsetMidY = yDist - offsetJ + UNSKEW_FACTOR_2D;
            float offsetLastX = xDist - 1.0 + 2.0 * UNSKEW_FACTOR_2D;
            float offsetLastY = yDist - 1.0 + 2.0 * UNSKEW_FACTOR_2D;
            
            int hash0 = i & 0xFF;
            int hash1 = j & 0xFF;
            int gradNdx0 = permutations[hash0 + permutations[hash1]] % 12;
            int gradNdx1 = permutations[hash0 + offsetI + permutations[hash1 + offsetJ]] % 12;
            int gradNdx2 = permutations[hash0 + 1 + permutations[hash1 + 1]] % 12;
            
            float t0 = 0.5 - xDist * xDist - yDist * yDist;
            float contrib0;
            if (t0 < 0.0) {
                contrib0 = 0.0;
            }
            else {
                t0 *= t0;
                contrib0 = t0 * t0 * dot(gradients + (gradNdx0 * 3), xDist, yDist);
            }
            
            float t1 = 0.5 - offsetMidX * offsetMidX - offsetMidY * offsetMidY;
            float contrib1;
            if (t1 < 0.0) {
                contrib1 = 0.0;
            }
            else {
                t1 *= t1;
                contrib1 = t1 * t1 * dot(gradients + (gradNdx1 * 3), offsetMidX, offsetMidY);
            }
            
            float t2 = 0.5 - offsetLastX * offsetLastX - offsetLastY * offsetLastY;
            float contrib2;
            if (t2 < 0.0) {
                contrib2 = 0.0;
            }
            else {
                t2 *= t2;
                contrib2 = t2 * t2 * dot(gradients + (gradNdx2 * 3), offsetLastX, offsetLastY);
            }
            
            int curNdx = ndx++;
            arr[curNdx] += 70.0 * (contrib0 + contrib1 + contrib2) * amplitude;
        }
    }
}

float SimplexNoise::Sample(float x, float y, float scaleX, float scaleY)
{
    x = x * scaleX + xOrigin;
    y = y * scaleY + yOrigin;
    
    float s = (x + y) * SKEW_FACTOR_2D;
    int i = fastFloor(x + s);
    int j = fastFloor(y + s);
    
    float t = (i + j) * UNSKEW_FACTOR_2D;
    float x0 = i - t;
    float y0 = j - t;
    float xDist = x - x0;
    float yDist = y - y0;
    
    int offsetI;
    int offsetJ;
    if (xDist > yDist) {
        offsetI = 1;
        offsetJ = 0;
    }
    else {
        offsetI = 0;
        offsetJ = 1;
    }
    
    float offsetMidX = xDist - offsetI + UNSKEW_FACTOR_2D;
    float offsetMidY = yDist - offsetJ + UNSKEW_FACTOR_2D;
    float offsetLastX = xDist - 1.0 + 2.0 * UNSKEW_FACTOR_2D;
    float offsetLastY = yDist - 1.0 + 2.0 * UNSKEW_FACTOR_2D;
    
    int hash0 = i & 0xFF;
    int hash1 = j & 0xFF;
    int gradNdx0 = permutations[hash0 + permutations[hash1]] % 12;
    int gradNdx1 = permutations[hash0 + offsetI + permutations[hash1 + offsetJ]] % 12;
    int gradNdx2 = permutations[hash0 + 1 + permutations[hash1 + 1]] % 12;
    
    float t0 = 0.5 - xDist * xDist - yDist * yDist;
    float contrib0;
    if (t0 < 0.0) {
        contrib0 = 0.0;
    }
    else {
        t0 *= t0;
        contrib0 = t0 * t0 * dot(gradients + (gradNdx0 * 3), xDist, yDist);
    }
    
    float t1 = 0.5 - offsetMidX * offsetMidX - offsetMidY * offsetMidY;
    float contrib1;
    if (t1 < 0.0) {
        contrib1 = 0.0;
    }
    else {
        t1 *= t1;
        contrib1 = t1 * t1 * dot(gradients + (gradNdx1 * 3), offsetMidX, offsetMidY);
    }
    
    float t2 = 0.5 - offsetLastX * offsetLastX - offsetLastY * offsetLastY;
    float contrib2;
    if (t2 < 0.0) {
        contrib2 = 0.0;
    }
    else {
        t2 *= t2;
        contrib2 = t2 * t2 * dot(gradients + (gradNdx2 * 3), offsetLastX, offsetLastY);
    }
    
    return 70.0 * (contrib0 + contrib1 + contrib2);
}

SimplexNoiseOctaves::SimplexNoiseOctaves(Random& random, int octaves)
{
    m_Octaves = octaves;
    generatorCollection.resize(octaves);

    for (int var3 = 0; var3 < octaves; ++var3) {
        generatorCollection[var3] = SimplexNoise(random);
    }
}

float SimplexNoiseOctaves::Sample(int localX, int localZ, float scaleX, float scaleZ, float lacunarity, float persistence)
{
    scaleX /= 1.5f;
    scaleZ /= 1.5f;

    float amplitude = 1.0f;
    float frequency = 1.0f;
    float total = 0.0f;

    for (int i = 0; i < m_Octaves; ++i) {
        total += generatorCollection[i].Sample(localX, localZ, scaleX * frequency, scaleZ * frequency) * (0.55f / amplitude);
        frequency *= lacunarity;
        amplitude *= persistence;
    }

    return total;
}

float* SimplexNoiseOctaves::SampleBiome2D(float* noiseArray, int localX, int localZ, int sizeX, int sizeZ, float scaleX, float scaleZ, float lacunarity, float persistence)
{
    scaleX /= 1.5f;
    scaleZ /= 1.5f;
    if (noiseArray == NULL) {
        noiseArray = new float[sizeX * sizeZ];
    }
    else {
        auto size = sizeX * sizeZ;
        for (int i = 0; i < size; ++i) {
            noiseArray[i] = 0.0f;
        }
    
    
    }
    auto size = sizeX * sizeZ;
    for (int i = 0; i < size; ++i)
    {
        float amplitude = 1.0f;
        float frequency = 1.0f;
        float total = 0.0f;

        int x = localX + (i % sizeX);
        int z = localZ + (i / sizeZ);

        for (int i = 0; i < m_Octaves; ++i) {
            total += generatorCollection[i].Sample(x, z, scaleX * frequency, scaleZ * frequency) * (0.55f / amplitude);
            frequency *= lacunarity;
            amplitude *= persistence;
        }

        noiseArray[i] = total;
    }

    return noiseArray;
}
