#pragma once

namespace Velox {

struct TextDrawStyle;

void DrawPerformanceStats();

void DrawMemoryUsageStats();

void DrawSettings();

void TextStyleEditor(Velox::TextDrawStyle* style, bool useCurrentAsBase = false);

void UpdateFrameHistory();

}
