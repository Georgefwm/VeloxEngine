#pragma once

namespace Velox {

struct TextDrawInfo {
    vec3 position = vec3(100.0, 100.0, 0);
    unsigned int textSize = 48;
    vec4 color = vec4(1.0);
};

void PushFont(const char* fontName);
void PopFont();

void DrawText(const char* text, TextDrawInfo fontDrawInfo);

void InitText();

}
