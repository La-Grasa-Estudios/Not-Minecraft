#include "Font.h"

#include "engine/System.h"
#include "engine/RenderingInterface.h"

#include "client/renderer/VertexProducer.h"
#include "client/ResourceManager.h"
#include "client/Client.h"

#include <fstream>
#include <iostream>
#include <string>

static riDevice* g_Device;
static VertexProducer g_Producer;

void Font::Init(riDevice* device) {

    g_Device = device;

    int w = 0;
    int h = 0;
    int c = 0;

    std::ifstream file(sysPath("mc/textures/default.rtex"), std::ios::binary);
    file.read((char*)&w, 4);
    file.read((char*)&h, 4);
    file.read((char*)&c, 4);

    w = bswap32(w);
    h = bswap32(h);
    c = bswap32(c);

    size_t sz = w * h * c;
    int* rawPixels = new int[w * h];
    file.read((char*)rawPixels, sz);

    for (int i = 0; i < 128; ++i) {
        int xt = i % 16;
        int yt = i / 16;
        int x = 0;
        for (bool emptyColumn = false; x < 8 && !emptyColumn; ++x) {
            int xPixel = xt * 8 + x;
            emptyColumn = true;
            for (float y = 0; y < 8 && emptyColumn; ++y) {
                int yPixel = yt * 8 + y;
                int pixel = bswap32(rawPixels[xPixel + yPixel * w]) & 0xFF;
                if (pixel > 128) {
                    emptyColumn = false;
                }
            }
        }
        if (i == 32) {
            x = 4;
        }
        charWidths[i] = x;
    }

    delete[] rawPixels;

}

void Font::_drawShadow(std::string_view str, float x, float y, int color)
{
    _draw(str, x + 1, y + 1, color, true);
    _draw(str, x, y, color, false);
}

void Font::DrawShadow(std::string_view str, float x, float y, int color)
{
    Draw(str, x + 1, y + 1, color, true);
    Draw(str, x, y, color, false);
}

void Font::DrawCenteredString(std::string_view str, float x, float y, int color)
{
    auto width = Width(str);
    x -= width / 2.0f;
    Draw(str, x + 1, y + 1, color, true);
    Draw(str, x, y, color, false);
}

void Font::_draw(std::string_view str, float x, float y, int color, bool darken)
{
    if (darken) {
        color = (color & 0xFCFCFC) >> 2;
    }

    g_Producer.SetColor(color);

    float xo = 0;
    for (int i = 0; i < str.length(); ++i) {
        if (str.at(i) == '&' && i + 2 < str.length()) {
            int cc =(int)std::string("0123456789abcdef").find(str.at(i + 1));
            int br = (cc & 0x8) * 8;
            int b = (cc & 0x1) * 191 + br;
            int g = ((cc & 0x2) >> 1) * 191 + br;
            int r = ((cc & 0x4) >> 2) * 191 + br;
            color = (r << 16 | g << 8 | b);
            i += 2;
            if (darken) {
                color = (color & 0xFCFCFC) >> 2;
            }
            g_Producer.SetColor(color);
        }
        char ch = str.at(i);
        int ix = ch % '\u0010' * 8;
        int iy = ch / '\u0010' * 8;
        g_Producer.AddVertex((float)(x + xo), (float)(y + 8), 0.0f, ix / 128.0f, (iy + 8) / 128.0f);
        g_Producer.AddVertex((float)(x + xo + 8), (float)(y + 8), 0.0f, (ix + 8) / 128.0f, (iy + 8) / 128.0f);
        g_Producer.AddVertex((float)(x + xo + 8), (float)y, 0.0f, (ix + 8) / 128.0f, iy / 128.0f);
        g_Producer.AddVertex((float)(x + xo), (float)y, 0.0f, ix / 128.0f, iy / 128.0f);
        xo += charWidths[ch];
    }

    
}

void Font::Draw(std::string_view str, float x, float y, int color, bool darken) {
    g_Device->SetTexturing(true);
    Client::GetInstance().pResourceManager->BindTexture(E_TextureResource_Font, g_Device);

    g_Producer.Reset();
    _draw(str, x, y, color, darken);

    g_Device->Draw(g_Producer.GetVertexPointer(), 0, RI_PRIMITIVE_TYPE_QUAD, g_Producer.GetVertexCount());
    g_Device->SetTexturing(false);
}

int Font::Width(std::string_view str)
{
    int len = 0;
    for (int i = 0; i < str.length(); ++i) {
        if (str.at(i) == '&') {
            ++i;
        }
        else {
            len += charWidths[str.at(i)];
        }
    }
    return len;
}
