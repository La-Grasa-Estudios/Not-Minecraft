#include "AnimatedWaterTexture.h"
#include "common/Random.h"
#include <string>
#include <cstring>
#include <cstdint>
#include <cstdlib>

// Cleaned up by Gemini, its 2am and i don't have the brain power to do this

void AnimatedWaterTexture::SetPixel(int x, int y, float r, float g, float b, float a)
{
    uint8_t R = static_cast<uint8_t>(r * 255.0f);
    uint8_t G = static_cast<uint8_t>(g * 255.0f);
    uint8_t B = static_cast<uint8_t>(b * 255.0f);
    uint8_t A = static_cast<uint8_t>(a * 255.0f);
    auto rgba = (A << 24) | (B << 16) | (G << 8) | (R);
    Buffer[x + y * 16] = rgba;
}

AnimatedWaterTexture::AnimatedWaterTexture()
{
    std::fill(std::begin(impulseGrid), std::end(impulseGrid), 0.0f);
    std::fill(std::begin(velocityGrid), std::end(velocityGrid), 0.0f);
    std::fill(std::begin(tempSimulationGrid), std::end(tempSimulationGrid), 0.0f);
    std::fill(std::begin(currentRippleGrid), std::end(currentRippleGrid), 0.0f);
    std::fill(std::begin(Buffer), std::end(Buffer), 0);
}

void AnimatedWaterTexture::Animate()
{
    for (int x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            float neighborRippleSum = 0.0f;

            for (int offsetX = x - 1; offsetX <= x + 1; offsetX++)
            {
                int wrappedX = offsetX & 0xf;
                int wrappedY = y & 0xf;
                neighborRippleSum += currentRippleGrid[wrappedX + wrappedY * 16];
            }

            tempSimulationGrid[x + y * 16] = neighborRippleSum / 3.3f + velocityGrid[x + y * 16] * 0.8f;
        }
    }

    for (int x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            const int index = x + y * 16;
            velocityGrid[index] += impulseGrid[index] * 0.05f;
            if (velocityGrid[index] < 0.0f)
            {
                velocityGrid[index] = 0.0f;
            }
            impulseGrid[index] -= 0.1f;
            if (Random::GlobalRand.NextFloat() < 0.05f)
            {
                impulseGrid[index] = 0.5f;
            }
        }
    }

    float swapBuffer[256];
    std::memcpy(swapBuffer, tempSimulationGrid, sizeof(float) * 256);
    std::memcpy(tempSimulationGrid, currentRippleGrid, sizeof(float) * 256);
    std::memcpy(currentRippleGrid, swapBuffer, sizeof(float) * 256);

    for (int i = 0; i < 256; i++)
    {
        float rippleIntensity = currentRippleGrid[i];
        if (rippleIntensity > 1.0f)
        {
            rippleIntensity = 1.0f;
        }
        if (rippleIntensity < 0.0f)
        {
            rippleIntensity = 0.0f;
        }
        float squaredIntensity = rippleIntensity * rippleIntensity;
        int r = (int)(32.0f + squaredIntensity * 32.0f);
        int g = (int)(50.0f + squaredIntensity * 64.0f);
        int b = 255;
        int a = (int)(200.0f + squaredIntensity * 54.0f);
        Buffer[i * 4 + 0] = (uint8_t)r;
        Buffer[i * 4 + 1] = (uint8_t)g;
        Buffer[i * 4 + 2] = (uint8_t)b;
        Buffer[i * 4 + 3] = (uint8_t)a;
    }
}

AnimatedWaterFlowTexture::AnimatedWaterFlowTexture()
{
    std::fill(std::begin(impulseGrid), std::end(impulseGrid), 0.0f);
    std::fill(std::begin(velocityGrid), std::end(velocityGrid), 0.0f);
    std::fill(std::begin(tempSimulationGrid), std::end(tempSimulationGrid), 0.0f);
    std::fill(std::begin(currentRippleGrid), std::end(currentRippleGrid), 0.0f);
    ticks = 0;
}

void AnimatedWaterFlowTexture::Animate()
{
    for (int x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            float neighborRippleSum = 0.0f;

            for (int offsetX = x - 1; offsetX <= x + 1; offsetX++)
            {
                int wrappedX = x & 0xf;
                int wrappedY = offsetX & 0xf;
                neighborRippleSum += currentRippleGrid[wrappedX + wrappedY * 16];
            }

            tempSimulationGrid[x + y * 16] = neighborRippleSum / 3.3f + velocityGrid[x + y * 16] * 0.8f;
        }
    }

    for (int x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            const int index = x + y * 16;
            velocityGrid[index] += impulseGrid[index] * 0.05f;
            if (velocityGrid[index] < 0.0f)
            {
                velocityGrid[index] = 0.0f;
            }
            impulseGrid[index] -= 0.1f;
            if (Random::GlobalRand.NextFloat() < 0.05f)
            {
                impulseGrid[index] = 0.5f;
            }
        }
    }

    float swapBuffer[256];
    std::memcpy(swapBuffer, tempSimulationGrid, sizeof(float) * 256);
    std::memcpy(tempSimulationGrid, currentRippleGrid, sizeof(float) * 256);
    std::memcpy(currentRippleGrid, swapBuffer, sizeof(float) * 256);

    ++ticks;

    for (int i = 0; i < 256; i++)
    {
        int flowOffsetIndex = (i - ticks * 16) & 0xFF;
        float rippleIntensity = currentRippleGrid[flowOffsetIndex];
        if (rippleIntensity > 1.0f)
        {
            rippleIntensity = 1.0f;
        }
        if (rippleIntensity < 0.0f)
        {
            rippleIntensity = 0.0f;
        }
        float squaredIntensity = rippleIntensity * rippleIntensity;
        int r = (int)(32.0f + squaredIntensity * 32.0f);
        int g = (int)(50.0f + squaredIntensity * 64.0f);
        int b = 255;
        int a = (int)(200.0f + squaredIntensity * 54.0f);
        Buffer[i * 4 + 0] = (uint8_t)r;
        Buffer[i * 4 + 1] = (uint8_t)g;
        Buffer[i * 4 + 2] = (uint8_t)b;
        Buffer[i * 4 + 3] = (uint8_t)a;
    }
}