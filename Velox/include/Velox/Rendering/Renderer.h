#pragma once

#include <SDL3/SDL_events.h>

struct SDL_Window;

namespace Velox {

struct Arena;

struct Rectangle {
    float x;
    float y;
    float w;
    float h;
};

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
    float textSize       = 24.0f;
    float fontWeightBias = 0.0f;  // Range (0.01f, 0.1f). Does effect char bounds.
    vec4  color          = COLOR_WHITE;
    float lineSpacing    = 1.0f;  // Scaler ((fontDefaultLineSpacing * fontLineHeight) * lineSpacing)
    vec4  outlineColor   = COLOR_BLACK;
    float outlineWidth   = 0.2f;  // Range (0.0f, 0.5f).
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
    std::string vertFilepath;
    std::string fragFilepath;
};

struct Pipeline;
struct DrawCommand {
    Velox::Pipeline* pipeline;
    ShaderProgram*   shader;
    Texture*         texture;
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

void DrawQuad(const mat4& transform, const mat4& uvTransform, const vec4& color,
        Velox::Texture* texture = nullptr, Velox::ShaderProgram* shader = nullptr);

void DrawQuad(const vec3& position, const vec2& size, const vec4& color,
        Velox::Texture* texture = nullptr, Velox::ShaderProgram* shader = nullptr);

void DrawRotatedQuad(const vec3& position, const vec2& size, const vec4& color, 
        const f32& rotation, Velox::Texture* texture = nullptr, Velox::ShaderProgram* shader = nullptr);

// inRect defines uv positions in uv space (0.0f, 1.0f).
void DrawQuadUV(const Velox::Rectangle& outRect, const Velox::Rectangle& inRect, 
        const vec4& color, Velox::Texture* texture = nullptr, Velox::ShaderProgram* shader = nullptr);

void DrawLine(const vec3& p0, const vec3& p1, const vec4& color);

void DrawRect(const Velox::Rectangle& rect, const vec4& color);
void DrawRect(const vec3& position, const vec2& size, const vec4& color);

TextContinueInfo DrawText(const char* text, const vec3& position, TextContinueInfo* textContinueInfo = nullptr);

Velox::TextContinueInfo DrawColoredText(const char* text, const vec3& position,
        const vec4& color, Velox::TextContinueInfo* textContinueInfo = nullptr);

}
