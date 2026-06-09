#include "Time.h"
#include "System.h"

#include <chrono>

void Time::BeginProfile()
{
	now = sysTime();
}

void Time::EndProfile()
{
	DeltaTime = (float)(sysTime() - now);
	UnscaledDeltaTime = DeltaTime;
	DeltaTime *= TimeScale;
	if (SkipFrame)
	{
		SkipFrame = false;
		DeltaTime = 0.0f;
		UnscaledDeltaTime = 0.0f;
	}
	GlobalTime += DeltaTime;
}

void Time::BeginRender()
{
}

void Time::EndRender()
{
}

void Time::ClearUpdate()
{
	UpdateTime = 0.0f;
}
