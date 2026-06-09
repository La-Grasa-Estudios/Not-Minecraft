#pragma once
class ScaledResolution
{
private:

	double scaledWidthD;
	double scaledHeightD;
	int scaledWidth;
	int scaledHeight;

public:
	ScaledResolution(int width, int height, int scaleFactor);
	double getOrthoWidth();
	double getOrthoHeight();
	int getScreenWidth();
	int getScreenHeight();
};

