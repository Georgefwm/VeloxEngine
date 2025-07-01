#pragma once

namespace Velox {

struct Font;
struct TextDrawStyle;
struct TextContinueInfo;
struct Rectangle;

void pushFont(const char* fontName);
void popFont();
Velox::Font* GetUsingFont();

void pushTextStyle(const Velox::TextDrawStyle& style);
void popTextStyle();
Velox::TextDrawStyle* GetUsingTextStyle();

// Get the continues info for a given string.
void getStringContinueInfo(const char* text, Velox::TextContinueInfo* resultInfo,
        Velox::TextContinueInfo* startInfo = nullptr);

void getStringBounds(const char* text, Velox::Rectangle* bounds,
        Velox::TextContinueInfo* startInfo = nullptr);

void initText();

}
