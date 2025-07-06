#include "UI.h"
#include <PCH.h>

#include "Arena.h"
#include "Rendering/Renderer.h"
#include "Event.h"

#include <SDL3/SDL.h>

#include <imgui.h>
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"


void Velox::initUI()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    SDL_Window* window = Velox::GetWindow();
    void* glContext = Velox::GetGLContext();

    ImGui_ImplSDL3_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init();

    float baseFontSize = 24.0f;
    float displayScale = Velox::getDisplayScale();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(displayScale);

    // io.FontGlobalScale = displayScale;

    ImFontConfig fontConfig {};
    fontConfig.RasterizerDensity = displayScale;

    Velox::Arena tempData(2048);

    const size_t pathSize = 1024;
    char* absolutePath = tempData.alloc<char>(pathSize);

    SDL_strlcpy(absolutePath, SDL_GetBasePath(), pathSize);
    SDL_strlcat(absolutePath, "assets\\fonts\\", pathSize);
    SDL_strlcat(absolutePath, "commit_mono.ttf", pathSize);

    ImFont* commitMonoFont = 
        io.Fonts->AddFontFromFileTTF(absolutePath, baseFontSize * displayScale, &fontConfig);

    // Begin first frame state.
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    Velox::SubscribeInfo subInfo {
        .name = "UI (+ imgui)",
        // Leaving this blank for now, will see how it behaves.
        //.eventRangeStart = u32(0x200), // SDL input event range.
        //.eventRangeEnd   = u32(0x300),
        .callback = Velox::uiEventCallback,
        .priority = 3,
    };

    Velox::getEventPublisher()->subscribe(subInfo);
}

bool Velox::uiEventCallback(SDL_Event& event)
{ 
    ImGui_ImplSDL3_ProcessEvent(&event);
    return false;
}

ImDrawData* Velox::getUIDrawData()
{
    return ImGui::GetDrawData();
}

void Velox::deInitUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}
