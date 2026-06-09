#pragma once

class AnimatedPortalTexture
{
public:
	AnimatedPortalTexture();
	void Animate();
	int ticks = 0;
	unsigned char Buffer[256 * 4 * 32]; // 16 * 16 * 4 colors * 32 frames
	unsigned char GpuBuffer[256 * 4]; // 16 * 16 * 4 colors
};