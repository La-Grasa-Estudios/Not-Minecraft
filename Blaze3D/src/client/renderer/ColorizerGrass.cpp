#include "ColorizerGrass.h"

#include "engine/System.h"

#include <fstream>

static bool g_LoadedImage = false;

static int pixels[65536];

glm::vec3 ColorizerGrass::GetColor(float temperature, float humidity)
{
    if (!g_LoadedImage)
    {
        g_LoadedImage = true;

        int w, h, c;

        std::ifstream file(sysPath("mc/textures/grasscolor.rtex"), std::ios::binary);
        file.read((char*)&w, 4);
        file.read((char*)&h, 4);
        file.read((char*)&c, 4);

        w = bswap32(w);
        h = bswap32(h);
        c = bswap32(c);

        auto size = sizeof(pixels);

        file.read((char*)pixels, size);
    }

    int x = (int)((1.0f - temperature) * 255.0f);
    int y = (int)((1.0f - humidity) * 255.0f);

    int pixel = bswap32(pixels[(y << 8) | x]);

    uint8_t r = (pixel >> 0) & 0xFF;
    uint8_t g = (pixel >> 8) & 0xFF;
    uint8_t b = (pixel >> 16) & 0xFF;

    return glm::vec3(
        r / 255.0f,
        g / 255.0f,
        b / 255.0f
    );
}
