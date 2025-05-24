#include "UI.h"

#include "Renderer.h"
#include "Event.h"

#include <SDL3/SDL.h>

#include <imgui.h>
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

#include <cstdio>

void Velox::InitUI()
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    SDL_Window* window    = Velox::GetWindow();
    SDL_GPUDevice* device = Velox::GetDevice();

    if (device == nullptr)
    {
        printf("Error: Device not initialised: %s\n", SDL_GetError());
        return;
    }

    ImGui_ImplSDL3_InitForSDLGPU(window);

    ImGui_ImplSDLGPU3_InitInfo initInfo = {};
    initInfo.Device = device;
    initInfo.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(device, window);
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

    // GM: Account for display scaling!
    // (I can read the text now without putting my face in the screen)
    io.FontGlobalScale = Velox::GetDisplayScale();
}

void Velox::ForwardSDLEvent(Velox::Event* event)
{
    ImGui_ImplSDL3_ProcessEvent(&event->sdlEvent);
}

ImDrawData* Velox::GetUIDrawData()
{
    return ImGui::GetDrawData();
}

void Velox::DeInitUI()
{
    ImGui::DestroyContext();
}
