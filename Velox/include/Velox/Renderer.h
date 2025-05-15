#pragma once

struct SDL_Window;
struct SDL_GPUDevice;

namespace Velox {

SDL_Window* GetWindow();

SDL_GPUDevice* GetDevice();

void InitRenderer();

void StartFrame();

void EndFrame();

void DoRenderPass();

void DeInitRenderer();

}
