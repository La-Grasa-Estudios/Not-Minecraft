#include "BiomeProvider.h"

#include "BiomeBase.h"
#include "common/block/Block.h"

#include <thirdparty/glm/ext.hpp>

static Biome* rainForest = new Biome(BLOCK_GRASS, BLOCK_DIRT, 5, 0, false);
static Biome* swampland = new Biome(BLOCK_GRASS, BLOCK_DIRT, 5, 0, false);
static Biome* seasonalForest = new Biome(BLOCK_GRASS, BLOCK_DIRT, 2, 0, false);
static Biome* forest = new Biome(BLOCK_GRASS, BLOCK_DIRT, 5, 0, false);
static Biome* savanna = new Biome(BLOCK_GRASS, BLOCK_DIRT, 0, 0, false);
static Biome* shrubland = new Biome(BLOCK_GRASS, BLOCK_DIRT, 0, 0, false);
static Biome* taiga = new Biome(BLOCK_GRASS, BLOCK_DIRT, 5, 0, true);
static Biome* desert = new Biome(BLOCK_SAND, BLOCK_SAND, 0, 10, false);
static Biome* plains = new Biome(BLOCK_GRASS, BLOCK_DIRT, -1, 0, false);
static Biome* iceDesert = new Biome(BLOCK_SAND, BLOCK_SAND, -1, 10, true);
static Biome* tundra = new Biome(BLOCK_GRASS, BLOCK_DIRT, -1, 0, true);

static Biome* GetBiome(float temperature, float humidity)
{
    humidity *= temperature;

    if (temperature < 0.1F) {
        return tundra;
    }

    if (humidity < 0.2F) {
        if (temperature < 0.5F) {
            return tundra;
        }
        else if (temperature < 0.95F) {
            return savanna;
        }
        else {
            return desert;
        }
    }

    if (humidity > 0.5F && temperature < 0.7F) {
        return swampland;
    }

    if (temperature < 0.5F) {
        return taiga;
    }
    else if (temperature < 0.97F) {
        if (humidity < 0.35F) {
            return shrubland;
        }
        else {
            return forest;
        }
    }
    else {
        if (humidity < 0.45F) {
            return plains;
        }
        else if (humidity < 0.9F) {
            return seasonalForest;
        }
        else {
            return rainForest;
        }
    }
}

static Biome* g_BiomeLut[4096];

BiomeProvider::BiomeProvider(uint64_t seed)
{
    auto temp = Random(seed * 9871ULL);
    auto hum = Random(seed * 39811ULL);
    auto mod = Random(seed * 543321ULL);
    m_GeneratorTemperature = SimplexNoiseOctaves(temp, 4);
    m_GeneratorHumidity = SimplexNoiseOctaves(hum, 4);
    m_GeneratorModifier = SimplexNoiseOctaves(mod, 2);
    for (int x = 0; x < 64; x++)
    {
        for (int y = 0; y < 64; y++)
        {
            g_BiomeLut[x + y * 64] = GetBiome(x / 63.0f, y / 63.0f);
        }
    }

    rainForest->Name = "Rain Forest";
    swampland->Name = "Swampland";
    seasonalForest->Name = "Seasonal Forest";
    forest->Name = "Forest";
    savanna->Name = "Savanna";
    shrubland->Name = "Shrubland";
    taiga->Name = "Taiga";
    desert->Name = "Desert";
    plains->Name = "Plains";
    iceDesert->Name = "Ice Desert";
    tundra->Name = "Tundra";
}

float BiomeProvider::GetTemperature(int x, int z)
{
    return m_TemperatureFinal[x * 16 + z];
}

float BiomeProvider::GetHumidity(int x, int z)
{
    return m_HumidityFinal[x * 16 + z];
}

float BiomeProvider::GetTemperatureForColumn(int x, int z)
{
    const float tempScale = 0.025f;
    const float modifierNoiseScale = 0.25f;

    float temperature = m_GeneratorTemperature.Sample(x, z, tempScale, tempScale, 0.25f, 0.5f);
    float modifier = m_GeneratorModifier.Sample(x, z, modifierNoiseScale, modifierNoiseScale, 0.588f, 0.5f);

    float modifierScale = 0.01f;
    float invModifierScale = 1.0f - modifierScale;
    temperature = (temperature * 0.15f + 0.7f) * invModifierScale + modifier * modifierScale;
    temperature = 1.0f - (1.0f - temperature) * (1.0f * temperature);

    temperature = glm::clamp(temperature, 0.0f, 1.0f);

    return temperature;
}

float BiomeProvider::GetHumidityForColumn(int x, int z)
{
    const float humidityScale = 0.05f;
    const float modifierNoiseScale = 0.25f;

    float humidity = m_GeneratorHumidity.Sample(x, z, humidityScale, humidityScale, 1.0f / 3.0f, 0.5f);
    float modifier = m_GeneratorModifier.Sample(x, z, modifierNoiseScale, modifierNoiseScale, 0.588f, 0.5f);

    float modifierScale = 0.01f;
    float invModifierScale = 1.0f - modifierScale;
    humidity = (humidity * 0.15f + 0.5f) * invModifierScale + modifier * modifierScale;
    humidity = glm::clamp(humidity, 0.0f, 1.0f);

    return humidity;
}

void BiomeProvider::GenerateValuesForChunk(int x, int z)
{
	int index = 0;
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
            index = i * 16 + j;
			m_TemperatureFinal[index] = GetTemperatureForColumn(i + x, j + z);
			m_HumidityFinal[index] = GetHumidityForColumn(i + x, j + z);
		}
	}
}

Biome* BiomeProvider::GetBiomeForColumn(int x, int z)
{
    return GetBiomeFromLut(m_TemperatureFinal[x * 16 + z], m_HumidityFinal[x * 16 + z]);
}

Biome* BiomeProvider::GetBiomeFromLut(float temperature, float humidity)
{
    int x = (int)(temperature * 63.0f);
    int y = (int)(humidity * 63.0f);
    return g_BiomeLut[x + y * 64];
}
