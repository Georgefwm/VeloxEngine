#include "Renderer.h"

#include "UI.h"

#include <glm/glm.hpp>
#include <SDL3/SDL_gpu.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include <cstdio>

SDL_Window*    g_window;
SDL_GPUDevice* g_device;

const ImVec4 g_clearColor = ImVec4(0.0, 0.0, 0.0, 1.0);

SDL_Window* Velox::GetWindow()    { return g_window; }
SDL_GPUDevice* Velox::GetDevice() { return g_device; }

void Velox::InitRenderer()
{
    SDL_WindowFlags windowFlags;
    windowFlags |= SDL_WINDOW_VULKAN;
    windowFlags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;

    
    g_window = SDL_CreateWindow("Velox App", 1920, 1080, windowFlags);
    if (g_window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return;
    }

    SDL_SetWindowPosition(g_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(g_window);

    // GM: Set shader format as vulkan (SPIR-V).
    SDL_GPUShaderFormat shaderFormat = SDL_GPU_SHADERFORMAT_SPIRV;

    g_device = SDL_CreateGPUDevice(shaderFormat, true, nullptr);

    if (g_device == nullptr)
    {
        printf("Error: SDL_CreateGPUDevice(): %s\n", SDL_GetError());
        return;
    }
    
    if (!SDL_ClaimWindowForGPUDevice(g_device, g_window))
    {
        printf("Error: SDL_ClaimWindowForGPUDevice(): %s\n", SDL_GetError());
        return;
    }

    // GM: Add option to change SDL_GPU_PRESENTMODE.
    // MAILBOX is garunteed to be supported.
    SDL_SetGPUSwapchainParameters(g_device, g_window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_MAILBOX);
}

void Velox::StartFrame()
{
    // GM: Not sure if should put the UI ones in UI.cpp or not, here should be fine for now.
    ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void Velox::EndFrame()
{
    // GM: Finalises and generates ImGui draw data.
    ImGui::Render();
}

void Velox::DoRenderPass()
{
    ImDrawData* drawData = Velox::GetUIDrawData();       

    // GM: Not sure if we need to check this, I thought when window is minimised then swapchainTexture == nullptr?
    const bool isMinimised = (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f);

    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(g_device);

    SDL_GPUTexture* swapchainTexture;
    SDL_AcquireGPUSwapchainTexture(commandBuffer, g_window, &swapchainTexture, nullptr, nullptr);

    if (swapchainTexture != nullptr && !isMinimised)
    {
        // This is mandatory: call ImGui_ImplSDLGPU3_PrepareDrawData() to upload the vertex/index buffer!
        Imgui_ImplSDLGPU3_PrepareDrawData(drawData, commandBuffer);

        // Setup and start a render pass
        SDL_GPUColorTargetInfo targetInfo = {};
        targetInfo.texture     = swapchainTexture;
        targetInfo.clear_color = 
            SDL_FColor { g_clearColor.x, g_clearColor.y, g_clearColor.z, g_clearColor.w };
        targetInfo.load_op     = SDL_GPU_LOADOP_CLEAR;
        targetInfo.store_op    = SDL_GPU_STOREOP_STORE;
        targetInfo.mip_level   = 0;
        targetInfo.cycle       = false;
        targetInfo.layer_or_depth_plane = 0;

        SDL_GPURenderPass* renderPass = 
            SDL_BeginGPURenderPass(commandBuffer, &targetInfo, 1, nullptr);

        // Render ImGui
        ImGui_ImplSDLGPU3_RenderDrawData(drawData, commandBuffer, renderPass);

        SDL_EndGPURenderPass(renderPass);
    }

    SDL_SubmitGPUCommandBuffer(commandBuffer);
}

void Velox::DeInitRenderer()
{
    SDL_WaitForGPUIdle(g_device);

    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLGPU3_Shutdown();

    SDL_ReleaseWindowFromGPUDevice(g_device, g_window);
    SDL_DestroyGPUDevice(g_device);
    SDL_DestroyWindow(g_window);
}

