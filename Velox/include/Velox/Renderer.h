#pragma once

#include <glad/gl.h>
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

struct Vertex {
    vec3 position;
    vec4 color;
    vec2 uv;
};

struct Texture {
    unsigned int id;
    void Use();
};

struct ShaderProgram {
    unsigned int id;
    void Use();
};

struct DrawCommand {
    ShaderProgram shader;
    Texture       texture;
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

ShaderProgram LoadShaderProgram(const char* vertexFilepath, const char* fragmentFilepath);

void DeInitRenderer();

Velox::Texture LoadTexture(const char* filepath, Velox::Arena* allocator);

void Draw(std::vector<Velox::Vertex>& vertices, std::vector<GLuint>& indices,
        unsigned int textureId, unsigned int shaderId = 0);


}
