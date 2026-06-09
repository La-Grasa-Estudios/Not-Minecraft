#include "AnimatedLavaTexture.h"
#include <string>
#include "common/Random.h"
#include "thirdparty/glm/ext.hpp"

constexpr int WAVE_FREQUENCY = 60;
constexpr int WAVE_RED_COLOR = 100;
constexpr int WAVE_GREEN_COLOR = 255;
constexpr int WAVE_BLUE_COLOR = 128;
constexpr int LAVA_ALPHA = 255;

// Cleaned up by Gemini, its 2am and i don't have the brain power to do this

AnimatedLavaTexture::AnimatedLavaTexture() {
    std::fill(std::begin(currentHeatGrid), std::end(currentHeatGrid), 0.0f);
    std::fill(std::begin(tempSimulationGrid), std::end(tempSimulationGrid), 0.0f);
    std::fill(std::begin(velocityGrid), std::end(velocityGrid), 0.0f);
    std::fill(std::begin(impulseGrid), std::end(impulseGrid), 0.0f);
}

void AnimatedLavaTexture::Animate() {
    for (int x = 0; x < 16; ++x) {
        for (int y = 0; y < 16; ++y) {
            float neighborHeatSum = 0.0f;
            for (int offsetX = x - 1; offsetX <= x + 1; ++offsetX) {
                for (int offsetY = y - 1; offsetY <= y + 1; ++offsetY) {
                    const int wrappedX = offsetX & 0xF;
                    const int wrappedY = offsetY & 0xF;
                    neighborHeatSum += currentHeatGrid[wrappedX + (wrappedY << 4)];
                }
            }
            float neighborVelocitySum = 0.0f;
            for (int offsetX = x; offsetX <= x + 1; ++offsetX) {
                for (int offsetY = y; offsetY <= y + 1; ++offsetY) {
                    const int wrappedX = offsetX & 0xF;
                    const int wrappedY = offsetY & 0xF;
                    neighborVelocitySum += velocityGrid[wrappedX + (wrappedY << 4)];
                }
            }
            tempSimulationGrid[x + (y << 4)] = neighborHeatSum / 9.0f * 0.9f + neighborVelocitySum / 4.0f * 0.8f;
        }
    }
    for (int x = 0; x < 16; ++x) {
        for (int y = 0; y < 16; ++y) {
            const int index = x + (y << 4);
            velocityGrid[index] += impulseGrid[index] * 0.01f;
            if (velocityGrid[index] < 0.0f) {
                velocityGrid[index] = 0.0f;
            }
            impulseGrid[index] -= 0.06f;
            if (Random::GlobalRand.NextFloat() < 5.0E-5f * WAVE_FREQUENCY + 0.0025f) {
                impulseGrid[index] = 1.5f;
            }
        }
    }
    float swapBuffer[256];
    std::memcpy(swapBuffer, currentHeatGrid, sizeof(swapBuffer));
    std::memcpy(currentHeatGrid, tempSimulationGrid, sizeof(swapBuffer));
    std::memcpy(tempSimulationGrid, swapBuffer, sizeof(swapBuffer));
    for (int i = 0; i < 256; ++i) {
        const float intensity = glm::max(0.0f, glm::min(1.0f, currentHeatGrid[i] * 2.0f));
        const int r = (int)(intensity * 100.0f + 155.0f);
        const int g = (int)(intensity * intensity * 255.0f);
        const int b = (int)(intensity * intensity * intensity * intensity * 128.0f);
        Buffer[(i << 2) + 0] = (uint8_t)r;
        Buffer[(i << 2) + 1] = (uint8_t)g;
        Buffer[(i << 2) + 2] = (uint8_t)b;
        Buffer[(i << 2) + 3] = 255;
    }
}

AnimatedLavaFlowTexture::AnimatedLavaFlowTexture() {
    std::fill(std::begin(currentHeatGrid), std::end(currentHeatGrid), 0.0f);
    std::fill(std::begin(tempSimulationGrid), std::end(tempSimulationGrid), 0.0f);
    std::fill(std::begin(velocityGrid), std::end(velocityGrid), 0.0f);
    std::fill(std::begin(impulseGrid), std::end(impulseGrid), 0.0f);
    ticks = 0;
}

void AnimatedLavaFlowTexture::Animate() {
    for (int x = 0; x < 16; ++x) {
        for (int y = 0; y < 16; ++y) {
            float neighborHeatSum = 0.0f;
            for (int offsetX = x - 1; offsetX <= x + 1; ++offsetX) {
                for (int offsetY = y - 1; offsetY <= y + 1; ++offsetY) {
                    const int wrappedX = offsetX & 0xF;
                    const int wrappedY = offsetY & 0xF;
                    neighborHeatSum += currentHeatGrid[wrappedX + (wrappedY << 4)];
                }
            }
            float neighborVelocitySum = 0.0f;
            for (int offsetX = x; offsetX <= x + 1; ++offsetX) {
                for (int offsetY = y; offsetY <= y + 1; ++offsetY) {
                    const int wrappedX = offsetX & 0xF;
                    const int wrappedY = offsetY & 0xF;
                    neighborVelocitySum += velocityGrid[wrappedX + (wrappedY << 4)];
                }
            }
            tempSimulationGrid[x + (y << 4)] = neighborHeatSum / 9.0f * 0.9f + neighborVelocitySum / 4.0f * 0.8f;
        }
    }
    for (int x = 0; x < 16; ++x) {
        for (int y = 0; y < 16; ++y) {
            const int index = x + (y << 4);
            velocityGrid[index] += impulseGrid[index] * 0.01f;
            if (velocityGrid[index] < 0.0f) {
                velocityGrid[index] = 0.0f;
            }
            impulseGrid[index] -= 0.06f;
            if (Random::GlobalRand.NextFloat() < 5.0E-5f * WAVE_FREQUENCY + 0.0025f) {
                impulseGrid[index] = 1.5f;
            }
        }
    }
    float swapBuffer[256];
    std::memcpy(swapBuffer, currentHeatGrid, sizeof(swapBuffer));
    std::memcpy(currentHeatGrid, tempSimulationGrid, sizeof(swapBuffer));
    std::memcpy(tempSimulationGrid, swapBuffer, sizeof(swapBuffer));

    ticks++;

    for (int i = 0; i < 256; ++i) {
        const int flowOffsetIndex = (i - (ticks / 3) * 16) & 0xFF;
        const float intensity = glm::max(0.0f, glm::min(1.0f, currentHeatGrid[flowOffsetIndex] * 2.0f));
        const int r = (int)(intensity * 100.0f + 155.0f);
        const int g = (int)(intensity * intensity * 255.0f);
        const int b = (int)(intensity * intensity * intensity * intensity * 128.0f);
        Buffer[(i << 2) + 0] = (uint8_t)r;
        Buffer[(i << 2) + 1] = (uint8_t)g;
        Buffer[(i << 2) + 2] = (uint8_t)b;
        Buffer[(i << 2) + 3] = 255;
    }
}