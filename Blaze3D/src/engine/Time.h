#pragma once

#include <cstdint>
#include <cstddef>

class Time {

	inline static float now = 0L;
	inline static float now_cpu = 0L;
	inline static float now_gpu = 0L;
	inline static float now_update = 0L;

public:

	inline static float Jitter = 0.0F;
	inline static float DeltaTime = 0.0F;
	inline static float GlobalTime = 0.0F; // Aproximate Time Since Startup
	inline static float FixedDeltaTime = 1.0F / 50.0F;
	inline static float TimeScale = 1.0f;
	inline static float UnscaledDeltaTime = 1.0f; // Unscaled delta time

	inline static float CPUTime = 0.0F;
	inline static float GPURenderTime = 0.0F;
	inline static float UpdateTime = 0.0F;

	inline static bool SkipFrame = false;

	static void BeginProfile();
	static void EndProfile();

	static void BeginRender();
	static void EndRender();

	static void ClearUpdate();

};
