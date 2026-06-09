#pragma once

#include <string_view>

class riDevice;

class Font
{
	static inline int charWidths[256];

	static void _drawShadow(std::string_view str, float x, float y, int color);
	static void _draw(std::string_view str, float x, float y, int color, bool darken);

public:
	
	static void Init(riDevice* device);
	static void DrawShadow(std::string_view str, float x, float y, int color);
	static void DrawCenteredString(std::string_view str, float x, float y, int color);
	static void Draw(std::string_view str, float x, float y, int color, bool darken);
	static int Width(std::string_view str);

};

