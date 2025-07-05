#pragma once

#include "Velox.h"

namespace Velox {

struct TextDrawStyle;

void drawPerformanceStats();

void drawMemoryUsageStats();

void drawSettings();

void drawEntityColliders();

void drawEntityHierarchyInfo();

VELOX_API void textStyleEditor(Velox::TextDrawStyle* style, bool useCurrentAsBase = false);

void updateFrameHistory();

}
