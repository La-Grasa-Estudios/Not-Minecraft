#pragma once

class AnimatedLavaTexture
{
	float currentHeatGrid[256];
	float tempSimulationGrid[256];
	float impulseGrid[256];
	float velocityGrid[256];
	void SetPixel(int x, int y, float r, float g, float b, float a);
public:
	AnimatedLavaTexture();
	void Animate();
	unsigned char Buffer[256 * 4]; // 16 * 16 * 4 colors
};

class AnimatedLavaFlowTexture
{
	float currentHeatGrid[256];
	float tempSimulationGrid[256];
	float impulseGrid[256];
	float velocityGrid[256];
	int ticks;
public:
	AnimatedLavaFlowTexture();
	void Animate();
	unsigned char Buffer[256 * 4]; // 16 * 16 * 4 colors
};