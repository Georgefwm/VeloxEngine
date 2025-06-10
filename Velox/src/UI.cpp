#include "UI.h"

#include "Renderer.h"
#include "Event.h"

#include <SDL3/SDL.h>

#include <imgui.h>
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"

#include <cstdio>

void Velox::InitUI()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    SDL_Window* window    = Velox::GetWindow();

    ImGui_ImplSDL3_InitForVulkan(window);
    ImGui_ImplVulkan_InitInfo init_info = Velox::GetImguiInitInfo();
    ImGui_ImplVulkan_Init(&init_info);

    // GM: Account for display scaling!
    // (I can read the text now without putting my face in the screen)
    io.FontGlobalScale = Velox::GetDisplayScale();
}

void Velox::ForwardSDLEventToUI(Velox::Event* event)
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
