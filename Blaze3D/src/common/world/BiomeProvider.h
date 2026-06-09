#pragma once

#include "common/noise/SimplexNoise.h"

#include <array>

class Biome;
class BiomeProvider
{
public:
	BiomeProvider(uint64_t seed);
	float GetTemperature(int x, int z);
	float GetHumidity(int x, int z);

	float GetTemperatureForColumn(int x, int z);
	float GetHumidityForColumn(int x, int z);

	void GenerateValuesForChunk(int x, int z);
	Biome* GetBiomeForColumn(int x, int z);
	Biome* GetBiomeFromLut(float temperature, float humidity);
private:
	std::array<float, 256> m_Temperature;
	std::array<float, 256> m_Humidity;
	std::array<float, 256> m_Modifier;

	std::array<float, 256> m_TemperatureFinal;
	std::array<float, 256> m_HumidityFinal;

	SimplexNoiseOctaves m_GeneratorTemperature;
	SimplexNoiseOctaves m_GeneratorHumidity;
	SimplexNoiseOctaves m_GeneratorModifier;
};