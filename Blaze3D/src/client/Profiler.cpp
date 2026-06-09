#include "Profiler.h"

#include <engine/System.h>
#include <engine/Time.h>

void Profiler::Clear()
{
	s_FinalizedSections = s_Sections;
	s_Sections.clear();
	s_CurrentTime = Time::GlobalTime;
}

void Profiler::BeginSection(const char* name)
{
	s_CurrentSection = name;
	s_CurrentTime = sysTime();
}

void Profiler::EndAndBeginSection(const char* name)
{
	EndSection();
	BeginSection(name);
}

void Profiler::EndSection()
{
	float dt = sysTime() - s_CurrentTime;
	s_Sections.emplace_back(s_CurrentSection, dt);
}

std::vector<ProfilerSection>& Profiler::GetSections()
{
	return s_FinalizedSections;
}
