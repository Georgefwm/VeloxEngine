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
};

SDL_Window* GetWindow();

SDL_GPUDevice* GetDevice();

bool InitRenderer();

void StartFrame();

void EndFrame();

void DoRenderPass();

void DeInitRenderer();

SDL_GPUShader* LoadShader(const char* filepath, SDL_GPUShaderStage shaderStage);

}
