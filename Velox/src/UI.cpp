#include "UI.h"
#include <PCH.h>

#include "Renderer.h"
#include "Event.h"

#include <SDL3/SDL.h>

#include <imgui.h>
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"


void Velox::InitUI()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    SDL_Window* window = Velox::GetWindow();
    SDL_GLContext* glContext = Velox::GetGLContext();

    ImGui_ImplSDL3_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init();
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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}
