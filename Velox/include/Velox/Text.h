#pragma once

namespace Velox {

struct Font;
struct TextDrawStyle;

void PushFont(const char* fontName);
void PopFont();
Velox::Font* GetUsingFont();

void PushTextStyle(const Velox::TextDrawStyle& style);
void PopTextStyle();
Velox::TextDrawStyle* GetUsingTextStyle();

void InitText();

}
