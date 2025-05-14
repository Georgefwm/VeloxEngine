#include "Velox.h"

#include <SDL3/SDL.h>
#include "SDL3/SDL_events.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include <cstdio>

bool g_quitRequested = false;

SDL_Window*    g_window   = nullptr;
SDL_GPUDevice* g_device   = nullptr;

// GM: Need decide on what vector types to use.
ImVec4 g_clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void Velox::test()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return;
    }

    SDL_WindowFlags windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
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

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLGPU(g_window);

    ImGui_ImplSDLGPU3_InitInfo initInfo = {};
    initInfo.Device = g_device;
    initInfo.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(g_device, g_window);
    initInfo.MSAASamples = SDL_GPU_SAMPLECOUNT_1;

    ImGui_ImplSDLGPU3_Init(&initInfo);

    // GM: From ImGui example:
    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Main loop.
    while (!g_quitRequested)
    {
        // Handle events.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);

            if (event.type == SDL_EVENT_QUIT)
                g_quitRequested = true;
        }
        
        // GM: Reduce framerate while window is minimised.
        if (SDL_GetWindowFlags(g_window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
        }

        // Update game state.
        // No game yet...

        // Render.
        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Render();

        ImDrawData* drawData = ImGui::GetDrawData();
        
        // GM: Not sure if we need to check this, thought when window is minimised then swapchainTexture == nullptr?
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

    // Cleanup.
    SDL_WaitForGPUIdle(g_device);

    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui::DestroyContext();

    SDL_ReleaseWindowFromGPUDevice(g_device, g_window);
    SDL_DestroyGPUDevice(g_device);
    SDL_DestroyWindow(g_window);

    SDL_Quit();    
}
