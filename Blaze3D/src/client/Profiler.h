#pragma once

#include <vector>

struct ProfilerSection
{
	const char* Name;
	float Duration;
};

class Profiler
{
public:
	static void Clear();
	static void BeginSection(const char* name);
	static void EndAndBeginSection(const char* name);
	static void EndSection();
	static std::vector<ProfilerSection>& GetSections();
private:
	static inline std::vector<ProfilerSection> s_Sections;
	static inline std::vector<ProfilerSection> s_FinalizedSections;
	static inline const char* s_CurrentSection = NULL;
	static inline float s_CurrentTime = 0.0f;
};