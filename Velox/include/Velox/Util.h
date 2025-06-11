#pragma once

#include <Types.h>
#include <imgui.h>

namespace Velox {

// Converts pixel coords to normalised shader coords (-1...1).
// Currently ignores z value because we aren't using it yet.
vec3 ToShaderCoords(vec3 vector, bool flipY);

inline ImVec4 ToImVec4(const glm::vec4& vec) { return ImVec4(vec.x, vec.y, vec.z, vec.w); }
// No ImVec3
inline ImVec2 ToImVec2(const glm::vec2& vec) { return ImVec2(vec.x, vec.y); }

float DeltaTime();

double DeltaTimePrecise();

float DeltaTimeMS();

float Time();

float RefreshDeltaTime();

}
