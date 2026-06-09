#pragma once
class Timer
{
private:
	const long NS_PER_SECOND = 1000000000L;
	const long MAX_NS_PER_UPDATE = 1000000000L;
	long long lastTime;
	float ticksPerSecond;
public:
	float a;
	float passedTime;
	int ticks;
	void advanceTime();
	Timer(float tickPerSecond);
};

