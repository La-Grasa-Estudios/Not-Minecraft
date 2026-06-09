#pragma once

class AnimatedWaterTexture
{
	float tempSimulationGrid[256];
	float velocityGrid[256];
	float impulseGrid[256];
	float currentRippleGrid[256];
	void SetPixel(int x, int y, float r, float g, float b, float a);
public:
	AnimatedWaterTexture();
	void Animate();
	unsigned char Buffer[256 * 4]; // 16 * 16 * 4 colors
};

class AnimatedWaterFlowTexture
{
	float tempSimulationGrid[256];
	float velocityGrid[256];
	float impulseGrid[256];
	float currentRippleGrid[256];
	int ticks;
public:
	AnimatedWaterFlowTexture();
	void Animate();
	unsigned char Buffer[256 * 4]; // 16 * 16 * 4 colors
};