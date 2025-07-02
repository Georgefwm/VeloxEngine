#pragma once

#include "Velox.h"

namespace Velox {

struct Font;
struct TextDrawStyle;
struct TextContinueInfo;
struct Rectangle;

VELOX_API void pushFont(const char* fontName);
VELOX_API void popFont();
VELOX_API Velox::Font* GetUsingFont();

VELOX_API void pushTextStyle(const Velox::TextDrawStyle& style);
VELOX_API void popTextStyle();
VELOX_API Velox::TextDrawStyle* GetUsingTextStyle();

// Get the continues info for a given string.
VELOX_API void getStringContinueInfo(const char* text, Velox::TextContinueInfo* resultInfo,
        Velox::TextContinueInfo* startInfo = nullptr);

VELOX_API void getStringBounds(const char* text, Velox::Rectangle* bounds,
        Velox::TextContinueInfo* startInfo = nullptr);

void initText();

}
