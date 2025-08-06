#pragma once

#include <Velox.h>

namespace Velox {

struct TextDrawStyle;

void drawPerformanceStats();

void drawMemoryUsageStats();

void drawSettings();

void drawEntityColliders();

void drawEntityHierarchyInfo();

VELOX_API void textStyleEditor(Velox::TextDrawStyle* style, bool useCurrentAsBase = false);

void floatScalerWidget(f32* value, const f32& min = 0.0f, const f32& max = 1.0f);

void updateFrameHistory();

}
