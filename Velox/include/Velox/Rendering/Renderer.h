#pragma once

#include "Velox.h"

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
    vec4  innerColor;
    vec2  uv;
    float threshold;
    float outBias;
    vec4  outerColor;
    float outlineWidthAbsolute;
    float outlineWidthRelative;
    float outlineBlur;
};

struct TextDrawStyle {
    float textSize       = 24.0f;
    float fontWeightBias = 0.5f;  // Range (0.0f, 1.0f)
    vec4  color          = COLOR_WHITE;
    float lineSpacing    = 1.0f;  // Scaler ((fontDefaultLineSpacing * fontLineHeight) * lineSpacing)
    vec4  outlineColor   = COLOR_BLACK;
    float outlineWidth   = 1.0f;  // Range (0.0f, 2.0f) for threshold == 0.5f. Range effected by threshold.
    float outlineBlur    = 1.0f;  // Range (0.0f, 2.0f). Very much depends on outlineWidth;

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
    void use();
};

struct ShaderProgram {
    u32  id;
    void use();
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

VELOX_API SDL_Window*    GetWindow();
VELOX_API SDL_GLContext* GetGLContext();

VELOX_API ivec2 getWindowSize();
VELOX_API i32 getVsyncMode();

VELOX_API f32 getDisplayScale();

VELOX_API void setResolution(ivec2 newResolution);
VELOX_API void setVsyncMode(int newMode);
VELOX_API bool isAdaptiveVsyncSupported();

void initRenderer();

bool forwardSDLEventToRenderer(SDL_Event* event);

void drawFrame();

VELOX_API void submitFrameData();

void doCopyPass();

void doRenderPass();

void deInitRenderer();

VELOX_API void drawQuad(const mat4& transform, const mat4& uvTransform, const vec4& color,
        Velox::Texture* texture = nullptr, Velox::ShaderProgram* shader = nullptr);

VELOX_API void drawQuad(const vec3& position, const vec2& size, const vec4& color,
        Velox::Texture* texture = nullptr, Velox::ShaderProgram* shader = nullptr);

VELOX_API void drawRotatedQuad(const vec3& position, const vec2& size, const vec4& color, 
        const f32& rotation, Velox::Texture* texture = nullptr, Velox::ShaderProgram* shader = nullptr);

// inRect defines uv positions in uv space (0.0f, 1.0f).
VELOX_API void drawQuadUV(const Velox::Rectangle& outRect, const Velox::Rectangle& inRect, 
        const vec4& color, Velox::Texture* texture = nullptr, Velox::ShaderProgram* shader = nullptr);

VELOX_API void drawLine(const vec3& p0, const vec3& p1, const vec4& color);

VELOX_API void drawRect(const Velox::Rectangle& rect, const vec4& color);
VELOX_API void drawRect(const vec3& position, const vec2& size, const vec4& color);

VELOX_API TextContinueInfo drawText(const char* text, const vec3& position, TextContinueInfo* textContinueInfo = nullptr);

VELOX_API Velox::TextContinueInfo drawColoredText(const char* text, const vec3& position,
        const vec4& color, Velox::TextContinueInfo* textContinueInfo = nullptr);

}
