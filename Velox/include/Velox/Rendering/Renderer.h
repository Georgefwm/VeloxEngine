#pragma once

#include <SDL3/SDL_events.h>

struct SDL_Window;

namespace Velox {

struct Arena;

struct UniformBufferObject {
    mat4  projection;
    mat4  view;
    ivec2 resolution;
    i32   padding[2]; // padding to match GLSL std140 layout
};

struct alignas(16) TextureVertex {
    vec3 position;
    vec4 color;
    vec2 uv;
};

struct alignas(16) LineVertex {
    vec3 position;
    vec4 color;
};

struct alignas(16) FontVertex {
    vec3  position;
    vec4  color;
    vec2  uv;
    float fontWeightBias;
    vec2  outlineColor;
    float outlineWidth;
    vec4  shadowColor;
    vec2  shadowOffset;
};

struct TextDrawStyle {
    float textSize       = 24;
    float fontWeightBias = 0.08f;  // Roughly in range (0.01f, 0.1f). Does effect char bounds.
    vec4  color          = COLOR_WHITE;
    float lineSpacing    = 1.0f;
    float outlineWidth   = 0.3f;  // Clamped to (0.0f, 0.5f).
    vec4  shadowColor    = COLOR_BLACK;
    vec2  shadowOffset   = vec2(0.0f, 0.0f);

    bool operator==(TextDrawStyle const& rhs) const
    {
        if (textSize != rhs.textSize) return false;
        if (color    != rhs.color   ) return false;
        return true;
    }
};

struct TextContinueInfo {
    char   lastChar;
    double advanceX;
    double advanceY;
};

struct Texture {
    u32  id;
    void Use();
};

struct ShaderProgram {
    u32  id;
    void Use();
};

struct Pipeline;
struct DrawCommand {
    Velox::Pipeline* pipeline;
    ShaderProgram    shader;
    Texture          texture;
    u32 indexOffset = 0;
    u32 numIndices  = 0;
};

SDL_Window*    GetWindow();
SDL_GLContext* GetGLContext();

ivec2 GetWindowSize();
i32 GetVsyncMode();

f32 GetDisplayScale();

void SetResolution(ivec2 newResolution);
void SetVsyncMode(int newMode);
bool IsAdaptiveVsyncSupported();

void InitRenderer();

bool ForwardSDLEventToRenderer(SDL_Event* event);

void StartFrame();

void DrawFrame();

void EndFrame();

void DoCopyPass();

void DoRenderPass();

void DeInitRenderer();

void DrawQuad(const mat4& transform, const vec4& color,
        const u32& textureID, const u32& shaderID = 0);

void DrawQuad(const vec3& position, const vec2& size, const vec4& color,
        const u32& textureID = 0, const u32& shaderID = 0);

void DrawLine(const vec3& p0, const vec3& p1, const vec4& color);

void DrawRect(const vec3& position, const vec2& size, const vec4& color);

TextContinueInfo DrawText(const char* text, const vec3& position, TextContinueInfo* textContinueInfo = nullptr);

Velox::TextContinueInfo DrawColoredText(const char* text, const vec3& position,
        const vec4& color, Velox::TextContinueInfo* textContinueInfo = nullptr);

}
