#pragma once

class AnimatedFireTexture
{
	float currentHeatGrid[320];
	float previousHeatGrid[320];
public:
	AnimatedFireTexture();
	void Animate();
	unsigned char Buffer[256 * 4]; // 16 * 16 * 4 colors
};