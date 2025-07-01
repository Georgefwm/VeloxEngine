#pragma once

#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

typedef glm::vec2 vec2;
typedef glm::ivec2 ivec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::mat4 mat4;

typedef int8_t   i8;
typedef uint8_t  u8;
typedef int16_t  i16;
typedef uint16_t u16;
typedef int32_t  i32;
typedef uint32_t u32;
typedef int64_t  i64;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;

constexpr vec4 COLOR_WHITE       = { 1.0f, 1.0f, 1.0f, 1.0f };
constexpr vec4 COLOR_BLACK       = { 0.0f, 0.0f, 0.0f, 1.0f };
constexpr vec4 COLOR_CLEAR       = { 0.0f, 0.0f, 0.0f, 0.0f };

constexpr vec4 COLOR_GRAY_LIGHT  = { 0.75f, 0.75f, 0.75f, 1.0f };
constexpr vec4 COLOR_GRAY_MEDIUM = { 0.50f, 0.50f, 0.50f, 1.0f };
constexpr vec4 COLOR_GRAY_DARK   = { 0.25f, 0.25f, 0.25f, 1.0f };

constexpr vec4 COLOR_RED         = { 0.89f, 0.26f, 0.20f, 1.0f }; // Tomato
constexpr vec4 COLOR_GREEN       = { 0.30f, 0.85f, 0.39f, 1.0f }; // Mint
constexpr vec4 COLOR_BLUE        = { 0.26f, 0.47f, 0.88f, 1.0f }; // Cornflower

constexpr vec4 COLOR_YELLOW      = { 1.00f, 0.92f, 0.23f, 1.0f }; // Sunflower
constexpr vec4 COLOR_ORANGE      = { 1.00f, 0.58f, 0.16f, 1.0f }; // Carrot
constexpr vec4 COLOR_PURPLE      = { 0.60f, 0.40f, 0.90f, 1.0f }; // Lavender

constexpr vec4 COLOR_CYAN        = { 0.00f, 0.78f, 0.89f, 1.0f };
constexpr vec4 COLOR_MAGENTA     = { 1.00f, 0.00f, 0.75f, 1.0f };

#define MS_PER_SECOND 1000