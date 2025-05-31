#pragma once

#include "Core.h"
#include "SDL3/SDL_gpu.h"

// struct SDL_Window;
// struct SDL_GPUDevice;
// struct SDL_GPUShader;

namespace Velox {

struct Vertex {
    vec3 position;
    vec4 color;
    vec2 uv;
};

SDL_Window* GetWindow();

SDL_GPUDevice* GetDevice();

ivec2 GetWindowSize();

float GetDisplayScale();

bool InitRenderer();

void StartFrame();

void EndFrame();

void DoRenderPass();

void DoCopyPass(SDL_GPUCommandBuffer* commandBuffer);

void DeInitRenderer();

SDL_GPUShader* LoadShader(const char* filepath, SDL_GPUShaderStage shaderStage, int numSamplers = 0, int numUniforms = 0);

SDL_Surface* LoadImage(const char* filepath);

// Returns the index of the vertex.
Uint32 AddVertex(Vertex vertex);

void AddIndex(Uint32 index);


}
