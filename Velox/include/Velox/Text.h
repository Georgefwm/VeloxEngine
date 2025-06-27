#pragma once

namespace Velox {

struct Font;
struct TextDrawStyle;
struct TextContinueInfo;

void PushFont(const char* fontName);
void PopFont();
Velox::Font* GetUsingFont();

void PushTextStyle(const Velox::TextDrawStyle& style);
void PopTextStyle();
Velox::TextDrawStyle* GetUsingTextStyle();

// Get the continues info for a given string.
void GetStringContinueInfo(const char* text, Velox::TextContinueInfo* resultInfo,
        Velox::TextContinueInfo* startInfo = nullptr);

void InitText();

}
