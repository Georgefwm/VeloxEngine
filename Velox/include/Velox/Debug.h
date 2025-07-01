#pragma once

namespace Velox {

struct TextDrawStyle;

void drawPerformanceStats();

void drawMemoryUsageStats();

void drawSettings();

void textStyleEditor(Velox::TextDrawStyle* style, bool useCurrentAsBase = false);

void updateFrameHistory();

}
