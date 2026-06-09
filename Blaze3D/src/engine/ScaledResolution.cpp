#include "ScaledResolution.h"

#include <math.h>

ScaledResolution::ScaledResolution(int i, int j, int k) {
	if (k == 0) {
		k = 1000;
	}

#ifdef __wii__
    const int factorX = 320;
#else
    const int factorX = 320;
#endif

    scaledWidth = i;
    scaledHeight = j;
    int scaleFactor = 1;
    for (; scaleFactor < k && scaledWidth / (scaleFactor + 1) >= factorX && scaledHeight / (scaleFactor + 1) >= 240; scaleFactor++) {}
    scaledWidthD = (double)scaledWidth / (double)scaleFactor;
    scaledHeightD = (double)scaledHeight / (double)scaleFactor;
    scaledWidth = (int)ceil(scaledWidthD);
    scaledHeight = (int)ceil(scaledHeightD);
}

double ScaledResolution::getOrthoWidth() {
    return scaledWidthD;
}

double ScaledResolution::getOrthoHeight() {
    return scaledHeightD;
}

int ScaledResolution::getScreenWidth() {
    return scaledWidth;
}
int ScaledResolution::getScreenHeight() {
    return scaledHeight;
}
