#include "MusicManager.h"

#include "common/Random.h"
#include "engine/Time.h"
#include "engine/Audio.h"
// In seconds
static float g_MusicUntil = 0.0f;

void MusicManager::Init()
{
	// 1200 seconds = 20 minutes of variation
	// That means g_MusicUntil = 10 minutes + [0-20] minutes
	g_MusicUntil = Random::GlobalRand.NextFloat() * 1200.0f + 600.0f;

	AudioEngine::RegisterMusic("calm", "mc/sound/music/calm1.mp3");
	AudioEngine::RegisterMusic("calm", "mc/sound/music/calm2.mp3");
	AudioEngine::RegisterMusic("calm", "mc/sound/music/calm3.mp3");
	AudioEngine::RegisterMusic("calm", "mc/sound/newmusic/hal1.mp3");
	AudioEngine::RegisterMusic("calm", "mc/sound/newmusic/hal2.mp3");
	AudioEngine::RegisterMusic("calm", "mc/sound/newmusic/hal3.mp3");
	AudioEngine::RegisterMusic("calm", "mc/sound/newmusic/hal4.mp3");
	AudioEngine::RegisterMusic("calm", "mc/sound/newmusic/nuance1.mp3");
	AudioEngine::RegisterMusic("calm", "mc/sound/newmusic/nuance2.mp3");
	AudioEngine::RegisterMusic("calm", "mc/sound/newmusic/piano1.mp3");
	AudioEngine::RegisterMusic("calm", "mc/sound/newmusic/piano2.mp3");
	AudioEngine::RegisterMusic("calm", "mc/sound/newmusic/piano3.mp3");
}

void MusicManager::Update()
{
	g_MusicUntil -= Time::DeltaTime;

	if (g_MusicUntil < 0.0f)
	{
		g_MusicUntil = Random::GlobalRand.NextFloat() * 1200.0f + 600.0f;
		if (!AudioEngine::IsMusicPlaying())
		{
			AudioEngine::PlayMusic("calm", 0.4f);
		}
	}
}
