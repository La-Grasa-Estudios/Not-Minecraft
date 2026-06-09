#include "Timer.h"
#include "engine/System.h"


Timer::Timer(float ticksPerSecond) {
	this->ticksPerSecond = ticksPerSecond;
	this->lastTime = sysTime() * 1000L * 1000L * 1000L;
	ticks = 0;
	a = 0;
	passedTime = 0;
}

void Timer::advanceTime() {
	long long now = sysTime() * 1000L * 1000L * 1000L;
	long long passedNs = now - lastTime;
	lastTime = now;

	if (passedNs < 0) passedNs = 0;
	if (passedNs > MAX_NS_PER_UPDATE) passedNs = MAX_NS_PER_UPDATE;

	passedTime += (float)passedNs * 1.0F * ticksPerSecond / 1.0E9F;

	ticks = (int)(passedTime);

	if (ticks > 200) ticks = 200;
	passedTime -= ticks;

	a = passedTime;
}
