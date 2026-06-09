#ifdef LEGACY_AUDIO

#include "Audio.h"
#define MA_NO_DEVICE_IO
#define MA_NO_THREADING
#define MA_NO_RUNTIME_LINKING
#define MA_NO_ATOMICS
#include "common/Random.h"
#include "System.h"

#include <vector>
#include <unordered_map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#ifdef PlaySound
#undef PlaySound
#endif

#ifdef __wii__
#define CHANNEL_COUNT 1
#else
#define CHANNEL_COUNT 2
#endif

extern std::string audioLog;

static std::unordered_map<std::string, std::vector<Mix_Chunk*>>  g_Sounds;
static std::unordered_map<std::string, std::vector<Mix_Music*>>  g_Music;

static Random g_AudRandom;
static glm::vec3 g_Position, g_Orientation;
static bool channelmap[16];
static bool channelmap3d[16];
static bool channelmap3ddynamic[16];
static glm::vec3 channelmapposition[16];
static glm::vec3* channelmappositionptr[16];

void AudioEngine::Init()
{
	g_AudRandom.SetSeed(time(NULL));
	Mix_OpenAudioDevice(32000, AUDIO_S16, CHANNEL_COUNT, 256, NULL, 0);
	Mix_AllocateChannels(16);
	memset(channelmap, 0, sizeof(channelmap));
	memset(channelmap3d, 0, sizeof(channelmap3d));
	memset(channelmap3ddynamic, 0, sizeof(channelmap3d));
}

void AudioEngine::Update()
{
	for (int i = 0; i < 16; i++)
	{
		if (channelmap[i])
		{
			if (!Mix_Playing(i))
			{
				channelmap[i] = false;
				continue;
			}
			if (channelmap3d[i])
			{
				int channel = i;

				auto position = channelmapposition[i];

				float distance = glm::clamp(glm::distance(channelmapposition[i], g_Position) / 16.0f, 0.0f, 1.0f);

				glm::vec2 v = glm::normalize(glm::vec2(channelmapposition[i].x, channelmapposition[i].z) - glm::vec2(g_Position.x, g_Position.z));
				float angle = (glm::atan(v.x, v.y) - glm::atan(g_Orientation.x, g_Orientation.z)) * 180.0f / glm::pi<float>();
				if (angle < 0.0f)
					angle += 360.0f;

				Mix_SetPosition(channel, static_cast<Sint16>(angle), static_cast<Uint8>(distance * 255.0f));
			}
		}
	}
}

void AudioEngine::RegisterSound(const std::string& name, const std::string& path)
{
	if (g_Sounds.find(name) == g_Sounds.end())
	{
		g_Sounds[name] = {};
	}
	auto& soundList = g_Sounds.at(name);
	auto chunk = Mix_LoadWAV(path.c_str());
	if (!chunk)
	{
		printf("Error loading %s: %s\n", path.c_str(), Mix_GetError());
	}
	soundList.push_back(chunk);
}

void AudioEngine::RegisterMusic(const std::string& name, const std::string& path)
{
	if (g_Music.find(name) == g_Music.end())
	{
		g_Music[name] = {};
	}
	auto& soundList = g_Music.at(name);
	auto chunk = Mix_LoadMUS(path.c_str());
	if (!chunk)
	{
		printf("Error loading %s: %s\n", path.c_str(), Mix_GetError());
	}
	soundList.push_back(chunk);
}

void AudioEngine::PlayMusic(const std::string& name, float volume)
{
	if (g_Music.find(name) == g_Music.end())
		return;
	auto& soundList = g_Music.at(name);
	auto sound = soundList[g_AudRandom.NextInt(soundList.size() * 100) % soundList.size()];
	if (sound)
	{
#ifdef __wii__
		volume *= 2.0f; // Thing is super quiet in the wii, boost
#endif
		Mix_PlayMusic(sound, 0);
		Mix_VolumeMusic((int)(glm::clamp(volume, 0.0f, 1.0f) * MIX_MAX_VOLUME));
	}
}

void AudioEngine::PlaySound(const std::string& name, float volume)
{
	if (g_Sounds.find(name) == g_Sounds.end())
		return;
	auto& soundList = g_Sounds.at(name);
	auto sound = soundList[g_AudRandom.NextInt(soundList.size()*100) % soundList.size()];
	if (sound)
	{
		sound->volume =(uint8_t)(glm::clamp(volume, 0.0f, 1.0f) * 128);
		Mix_PlayChannel(-1, sound, 0);
	}
}

int AudioEngine::PlaySound(const std::string& name, const glm::vec3& position, float volume)
{
	if (g_Sounds.find(name) == g_Sounds.end())
		return -1;
	auto& soundList = g_Sounds.at(name);
	auto sound = soundList[g_AudRandom.NextInt(soundList.size() * 100) % soundList.size()];
	if (sound)
	{
		sound->volume = (uint8_t)(glm::clamp(volume, 0.0f, 1.0f) * 128);
		float distance = glm::clamp(glm::distance(position, g_Position) / 16.0f, 0.0f, 1.0f);

		if (distance >= 0.99f)
		{
			return -1;
		}

		int channel = Mix_PlayChannel(-1, sound, 0);

		if (distance > 0.1f)
		{

			glm::vec2 v = glm::normalize(glm::vec2(position.x, position.z) - glm::vec2(g_Position.x, g_Position.z));
			float angle = (glm::atan(v.x, v.y) - glm::atan(g_Orientation.x, g_Orientation.z)) * 180.0f / glm::pi<float>();
			if (angle < 0.0f)
				angle += 360.0f;

			Mix_SetPosition(channel, static_cast<Sint16>(angle), static_cast<Uint8>(distance * 255.0f));
			channelmap[channel] = true;
			channelmap3d[channel] = true;
			channelmapposition[channel] = position;
		}

		return channel;
	}
	return -1;
}

bool AudioEngine::SetPosition(int id, const glm::vec3& position)
{
	if (channelmap[id])
	{
		channelmapposition[id] = position;
		return true;
	}
	return false;
}

bool AudioEngine::IsMusicPlaying()
{
	return Mix_PlayingMusic();
}

void AudioEngine::StopMusic()
{
	Mix_HaltMusic();
}

void AudioEngine::SetListener(glm::vec3 position, glm::vec3 orientation)
{
	g_Position = position;
	g_Orientation = orientation;
}

#else

#include "Audio.h"
#include "common/Random.h"
#include "System.h"

#include "miniaudio/miniaudio.h"

#include <vector>
#include <unordered_map>

#ifdef PlaySound
#undef PlaySound
#endif

#ifdef __wii__
#define CHANNEL_COUNT 1
#else
#define CHANNEL_COUNT 2
#endif

extern std::string audioLog;

static ma_engine g_Engine;

static std::unordered_map<std::string, std::vector<std::string>> g_Sounds;
static std::unordered_map<std::string, std::vector<std::string>> g_Music;

static Random g_AudRandom;
static glm::vec3 g_Position, g_Orientation;

static const int MAX_CHANNELS = 64;
static ma_sound g_Channels[MAX_CHANNELS];
static bool g_ChannelActive[MAX_CHANNELS];

static ma_sound g_MusicSound;
static bool g_MusicActive = false;

static int FindFreeChannel()
{
	for (int i = 0; i < MAX_CHANNELS; ++i) {
		if (!g_ChannelActive[i]) return i;
	}
	return -1;
}

void AudioEngine::Init()
{
	ma_result result = ma_engine_init(NULL, &g_Engine);
	if (result != MA_SUCCESS) {
		// Shit
	}
	ma_engine_listener_set_world_up(&g_Engine, 0, 0, 1, 0);

	for (int i = 0; i < MAX_CHANNELS; ++i) {
		g_ChannelActive[i] = false;
	}

	g_AudRandom.SetSeed(time(NULL));

	ma_engine_set_gain_db(&g_Engine, 4.0f);
}

void AudioEngine::Update()
{
	for (int i = 0; i < MAX_CHANNELS; ++i) {
		if (g_ChannelActive[i]) {
			if (ma_sound_at_end(&g_Channels[i])) {
				ma_sound_uninit(&g_Channels[i]);
				g_ChannelActive[i] = false;
			}
		}
	}
}

void AudioEngine::RegisterSound(const std::string& name, const std::string& path)
{
	g_Sounds[name].push_back(path);
}

void AudioEngine::RegisterMusic(const std::string& name, const std::string& path)
{
	g_Music[name].push_back(path);
}

void AudioEngine::PlayMusic(const std::string& name, float volume)
{
	if (g_Music.find(name) == g_Music.end() || g_Music[name].empty())
		return;

	auto& soundList = g_Music.at(name);
	const std::string& path = soundList[g_AudRandom.NextInt(soundList.size() * 100) % soundList.size()];

	if (g_MusicActive) {
		ma_sound_uninit(&g_MusicSound);
		g_MusicActive = false;
	}

	ma_result result = ma_sound_init_from_file(&g_Engine, path.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL, &g_MusicSound);
	if (result == MA_SUCCESS) {
		g_MusicActive = true;

#ifdef __wii__
		volume *= 2.0f; // Thing is super quiet in the wii, boost
#endif

		ma_sound_set_volume(&g_MusicSound, glm::clamp(volume, 0.0f, 1.0f));
		ma_sound_set_spatialization_enabled(&g_MusicSound, false); // 2D sound
		ma_sound_start(&g_MusicSound);
	}
}

void AudioEngine::PlaySound(const std::string& name, float volume)
{
	if (g_Sounds.find(name) == g_Sounds.end() || g_Sounds[name].empty())
		return;

	auto& soundList = g_Sounds.at(name);
	const std::string& path = soundList[g_AudRandom.NextInt(soundList.size() * 100) % soundList.size()];

	int channel = FindFreeChannel();
	if (channel != -1) {
		ma_result result = ma_sound_init_from_file(&g_Engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &g_Channels[channel]);
		if (result == MA_SUCCESS) {
			g_ChannelActive[channel] = true;
			ma_sound_set_spatialization_enabled(&g_Channels[channel], false);
			ma_sound_set_volume(&g_Channels[channel], glm::clamp(volume, 0.0f, 1.0f));
			ma_sound_start(&g_Channels[channel]);
		}
	}
}

int AudioEngine::PlaySound(const std::string& name, const glm::vec3& position, float volume)
{
	if (g_Sounds.find(name) == g_Sounds.end() || g_Sounds[name].empty())
		return -1;

	// Culling early based on distance
	float distance = glm::clamp(glm::distance(position, g_Position) / 16.0f, 0.0f, 1.0f);
	if (distance >= 0.99f) {
		return -1;
	}

	if (distance < 0.1f)
	{
		PlaySound(name, volume);
		return -1;
	}

	auto& soundList = g_Sounds.at(name);
	const std::string& path = soundList[g_AudRandom.NextInt(soundList.size() * 100) % soundList.size()];

	int channel = FindFreeChannel();
	if (channel != -1) {
		ma_result result = ma_sound_init_from_file(&g_Engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &g_Channels[channel]);
		if (result == MA_SUCCESS) {
			g_ChannelActive[channel] = true;

			ma_sound_set_spatialization_enabled(&g_Channels[channel], true);
			ma_sound_set_position(&g_Channels[channel], position.x, position.y, position.z);
			ma_sound_set_attenuation_model(&g_Channels[channel], ma_attenuation_model_linear);
			ma_sound_set_max_distance(&g_Channels[channel], 16.0f);
			ma_sound_set_volume(&g_Channels[channel], glm::clamp(volume, 0.0f, 1.0f));

			ma_sound_start(&g_Channels[channel]);
			return channel;
		}
	}
	return -1;
}

bool AudioEngine::SetPosition(int id, const glm::vec3& position)
{
	if (id >= 0 && id < MAX_CHANNELS && g_ChannelActive[id])
	{
		ma_sound_set_position(&g_Channels[id], position.x, position.y, position.z);
		return true;
	}
	return false;
}

bool AudioEngine::IsMusicPlaying()
{
	if (g_MusicActive) {
		return ma_sound_is_playing(&g_MusicSound);
	}
	return false;
}

void AudioEngine::StopMusic()
{
	if (g_MusicActive) {
		ma_sound_stop(&g_MusicSound);
		ma_sound_uninit(&g_MusicSound);
		g_MusicActive = false;
	}
}

void AudioEngine::SetListener(glm::vec3 position, glm::vec3 orientation)
{
	g_Position = position;
	g_Orientation = orientation;

	ma_engine_listener_set_position(&g_Engine, 0, position.x, position.y, position.z);
	ma_engine_listener_set_direction(&g_Engine, 0, -orientation.x, -orientation.y, -orientation.z);
}

#endif