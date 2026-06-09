#include "AnimatedFireTexture.h"

#include <string>
#include "common/Random.h"
#include "thirdparty/glm/ext.hpp"

// Cleaned up by Gemini, its 2am and i don't have the brain power to do this

AnimatedFireTexture::AnimatedFireTexture()
{
	memset(previousHeatGrid, 0, sizeof(float) * 256);
	memset(currentHeatGrid, 0, sizeof(float) * 256);
}

void AnimatedFireTexture::Animate() {
    const int GRID_WIDTH = 16;
    const int GRID_HEIGHT = 20;

    for (int x = 0; x < GRID_WIDTH; ++x) {
        for (int y = 0; y < GRID_HEIGHT; ++y) {

            int sampleCount = 18;
            float accumulatedHeat = currentHeatGrid[x + ((y + 1) % GRID_HEIGHT) * GRID_WIDTH] * sampleCount;

            for (int neighborX = x - 1; neighborX <= x + 1; ++neighborX) {
                for (int neighborY = y; neighborY <= y + 1; ++neighborY) {

                    if (neighborX >= 0 && neighborY >= 0 && neighborX < GRID_WIDTH && neighborY < GRID_HEIGHT) {
                        accumulatedHeat += currentHeatGrid[neighborX + neighborY * GRID_WIDTH];
                    }
                    ++sampleCount;
                }
            }

            previousHeatGrid[x + y * GRID_WIDTH] = accumulatedHeat / (sampleCount * 1.06f);

            if (y >= 19) {
                previousHeatGrid[x + y * GRID_WIDTH] = (float)(
                    Random::GlobalRand.NextFloat() * Random::GlobalRand.NextFloat() * Random::GlobalRand.NextFloat() * 4.0 +
                    Random::GlobalRand.NextFloat() * 0.1f + 0.2f
                    );
            }
        }
    }

    float tempSwapBuffer[GRID_WIDTH * GRID_HEIGHT];
    std::memcpy(tempSwapBuffer, previousHeatGrid, sizeof(tempSwapBuffer));
    std::memcpy(previousHeatGrid, currentHeatGrid, sizeof(tempSwapBuffer));
    std::memcpy(currentHeatGrid, tempSwapBuffer, sizeof(tempSwapBuffer));

    for (int i = 0; i < 256; ++i) {
        float heat = currentHeatGrid[i] * 1.8f;
        if (heat > 1.0f) heat = 1.0f;
        if (heat < 0.0f) heat = 0.0f;

        int redComponent = (int)(heat * 155.0f + 100.0f);
        int greenComponent = (int)(heat * heat * 255.0f);
        int blueComponent = (int)(heat * heat * heat * heat * heat * heat * heat * heat * heat * heat * 255.0f);

        unsigned char alphaComponent = 255;
        if (heat < 0.5f) {
            alphaComponent = 0;
        }

        Buffer[i * 4 + 0] = (unsigned char)redComponent;
        Buffer[i * 4 + 1] = (unsigned char)greenComponent;
        Buffer[i * 4 + 2] = (unsigned char)blueComponent;
        Buffer[i * 4 + 3] = alphaComponent;
    }
}
