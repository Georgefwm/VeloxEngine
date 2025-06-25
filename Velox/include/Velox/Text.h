#pragma once

namespace Velox {

struct Font;

void PushFont(const char* fontName);
void PopFont();
Velox::Font* GetUsingFont();

void InitText();

}
