#include "WorldHell.h"
#include "world/ChunkProviderHell.h"

WorldHell::WorldHell(const std::string& path) : World(path)
{
	for (int i = 0; i < 16; i++)
	{
		float a = 0.1f;
		float l = 1.0f - (float)i / 15.0f;
		LightLUT[i] = (1.0f - l) / (l * 3.0f + 1.0f) * (1.0f - a) + a;
	}
}

ChunkProvider& WorldHell::GetChunkProvider()
{
	if (!m_ChunkProvider)
	{
		m_ChunkProvider = new ChunkProviderHell(Seed);
	}
	return *m_ChunkProvider;
}

int WorldHell::GetDimensionID()
{
	return -1;
}

bool WorldHell::HasSky()
{
	return false;
}

void WorldHell::InitWorld()
{
	for (int i = 0; i < 16; i++)
	{
		float a = 0.15f;
		float l = 1.0f - (float)i / 15.0f;
		LightLUT[i] = (1.0f - l) / (l * 3.0f + 1.0f) * (1.0f - a) + a;
	}
}
