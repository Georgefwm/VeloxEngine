#pragma once

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <glm/matrix.hpp>

#include <imgui.h>

// GM: Typedef these for more consice use, do this because it should be the default datatype.
typedef glm::vec4 vec4;
typedef glm::vec3 vec3;
typedef glm::vec2 vec2;

typedef glm::ivec2 ivec2;

inline ImVec4 ToImVec4(const glm::vec4& vec) { return ImVec4(vec.x, vec.y, vec.z, vec.w); }
// No ImVec3
inline ImVec2 ToImVec2(const glm::vec2& vec) { return ImVec2(vec.x, vec.y); }

