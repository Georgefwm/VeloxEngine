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

struct TextDrawInfo {
    vec3 position = vec3(100.0, 100.0, 0);
    u32 textSize = 48;
    vec4 color = vec4(1.0);
};

struct TextureVertex {
    vec3 position;
    vec4 color;
    vec2 uv;
};

struct LineVertex {
    vec3 position;
    vec4 color;
};

struct FontVertex {
    vec3 position;
    vec4 color;
    vec2 uv;
    vec2 outlineColor;
};

struct Texture {
    u32 id;
    void Use();
};

struct ShaderProgram {
    u32 id;
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

f32 GetDisplayScale();

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

void DrawText(const char* text, const Velox::TextDrawInfo& textDrawInfo);
// void Draw(std::vector<Velox::Vertex>& vertices, std::vector<u32>& indices,
//       u32 textureId, u32 shaderId = 0);


}
