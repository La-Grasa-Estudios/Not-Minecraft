#pragma once

#include <string>
#include <thirdparty/glm/ext.hpp>

class AudioEngine
{
public:
	static void Init();
	static void Update();
	static void RegisterSound(const std::string& name, const std::string& path);
	static void RegisterMusic(const std::string& name, const std::string& path);
	static void PlayMusic(const std::string& name, float volume = 1.0f);
	static void PlaySound(const std::string& name, float volume = 1.0f);
	static int PlaySound(const std::string& name, const glm::vec3& position, float volume = 1.0f);
	static bool SetPosition(int id, const glm::vec3& position);
	static bool IsMusicPlaying();
	static void StopMusic();
	static void SetListener(glm::vec3 position, glm::vec3 orientation);
};