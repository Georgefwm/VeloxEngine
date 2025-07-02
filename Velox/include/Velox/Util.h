#pragma once

#include "Velox.h"

#include <imgui.h>

namespace Velox {

// Converts pixel coords to normalised shader coords (-1...1).
// Currently ignores z value because we aren't using it yet.
VELOX_API vec3 toShaderCoords(vec3 vector, bool flipY);

inline ImVec4 toImVec4(const glm::vec4& vec) { return ImVec4(vec.x, vec.y, vec.z, vec.w); }
// No ImVec3
inline ImVec2 toImVec2(const glm::vec2& vec) { return ImVec2(vec.x, vec.y); }

}
