#include "AnimatedPortalTexture.h"
#include "common/Random.h"
#include "thirdparty/glm/ext.hpp"

// Cleaned up by Gemini, its 2am and i don't have the brain power to do this

AnimatedPortalTexture::AnimatedPortalTexture() {
    Random prng(100);

    for (int frame = 0; frame < 32; ++frame) {
        for (int x = 0; x < 16; ++x) {
            for (int y = 0; y < 16; ++y) {
                float intensityMultiplier = 0.0f;
                int waveIndex;

                for (waveIndex = 0; waveIndex < 2; ++waveIndex) {
                    float centerOffsetIndexX = (float)(waveIndex * 8);
                    float centerOffsetIndexY = (float)(waveIndex * 8);

                    float normalizedX = ((float)x - centerOffsetIndexX) / 16.0f * 2.0f;
                    float normalizedY = ((float)y - centerOffsetIndexY) / 16.0f * 2.0f;

                    if (normalizedX < -1.0f) {
                        normalizedX += 2.0f;
                    }
                    if (normalizedX >= 1.0f) {
                        normalizedX -= 2.0f;
                    }
                    if (normalizedY < -1.0f) {
                        normalizedY += 2.0f;
                    }
                    if (normalizedY >= 1.0f) {
                        normalizedY -= 2.0f;
                    }

                    float radiusSquared = normalizedX * normalizedX + normalizedY * normalizedY;

                    float spiralAngle = (float)glm::atan((double)normalizedY, (double)normalizedX) +
                        ((float)frame / 32.0f * (float)glm::pi<float>() * 2.0f - radiusSquared * 10.0f + (float)(waveIndex * 2)) * (float)(waveIndex * 2 - 1);

                    spiralAngle = (glm::sin(spiralAngle) + 1.0f) / 2.0f;
                    spiralAngle /= radiusSquared + 1.0f;
                    intensityMultiplier += spiralAngle * 0.5f;
                }

                intensityMultiplier += prng.NextFloat() * 0.1f;

                int blueValue = (int)(intensityMultiplier * 100.0f + 155.0f);
                int redValue = (int)(intensityMultiplier * intensityMultiplier * 200.0f + 55.0f);
                int greenValue = (int)(intensityMultiplier * intensityMultiplier * intensityMultiplier * intensityMultiplier * 255.0f);
                int alphaValue = (int)(intensityMultiplier * 100.0f + 155.0f);

                int pixelIndex = y * 16 + x;
                int frameMemoryOffset = frame * 1024;

                Buffer[pixelIndex * 4 + 0 + frameMemoryOffset] = redValue;
                Buffer[pixelIndex * 4 + 1 + frameMemoryOffset] = greenValue;
                Buffer[pixelIndex * 4 + 2 + frameMemoryOffset] = blueValue;
                Buffer[pixelIndex * 4 + 3 + frameMemoryOffset] = alphaValue;
            }
        }
    }
}

void AnimatedPortalTexture::Animate() {
    ++ticks;
    unsigned char* framePointer = Buffer + ((ticks & 31) * 1024);

    for (int i = 0; i < 256; ++i) {
        int r = framePointer[i * 4 + 0] & 255;
        int g = framePointer[i * 4 + 1] & 255;
        int b = framePointer[i * 4 + 2] & 255;
        int a = framePointer[i * 4 + 3] & 255;

        GpuBuffer[i * 4 + 0] = r;
        GpuBuffer[i * 4 + 1] = g;
        GpuBuffer[i * 4 + 2] = b;
        GpuBuffer[i * 4 + 3] = a;
    }
}