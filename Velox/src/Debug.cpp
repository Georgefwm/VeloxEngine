#include "Debug.h"
#include <PCH.h>
#include <SDL3/SDL_video.h>

#include "Arena.h"
#include "Asset.h"
#include "Rendering/Renderer.h"
#include "Util.h"
#include "Velox.h"
#include "imgui.h"

constexpr size_t FRAME_HISTORY_COUNT = 1000;

static float s_frameTimeHistory[FRAME_HISTORY_COUNT];
static int  s_currentIndex    = FRAME_HISTORY_COUNT;
static bool s_capturingPaused = false;

void Velox::DrawPerformanceStats()
{
    if (!s_capturingPaused)
        Velox::UpdateFrameHistory();
    
    int count = 0;
    int validCount = 0; // Count w/ non-zero values (at the start of recording not all are set).
    float max = 0;
    float min = 10000;
    float sum = 0;
    float sumToSecond = 0;
    int i = s_currentIndex;
    int fps = 0;
    while (count <= FRAME_HISTORY_COUNT)
    {
        count += 1;

        if (i >= FRAME_HISTORY_COUNT)
            i = 0;

        float value = s_frameTimeHistory[i];

        i = (i + 1) % FRAME_HISTORY_COUNT;

        if (value <= 0)
            continue;

        validCount += 1;

        if (sumToSecond <= MS_PER_SECOND)
        {
            sumToSecond += value;
            fps += 1;
        }

        if (value > max)
            max = value;

        if (value < min)
            min = value;

        sum += value;    
    }

    float average = sum / validCount;

    // Draw UI.
    ImGuiWindowFlags flags = 0;
    
    ImGuiInputTextFlags inputTextFlags = 0;
    inputTextFlags |= ImGuiInputTextFlags_EnterReturnsTrue;
    inputTextFlags |= ImGuiInputTextFlags_CallbackCharFilter; 

    ImVec2 windowSize = { 500, 500 };

    // Don't change size of window, just position; To avoid resizing elements.
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(windowSize,  ImGuiCond_FirstUseEver);

    int valueSpacing = 300;

    bool shouldClosePerformanceStats = true;
    ImGui::Begin("Performance Stats", &shouldClosePerformanceStats, flags);
    if (!shouldClosePerformanceStats)
    {
        Velox::EngineState* engineState = Velox::GetEngineState();
        engineState->showPerformanceStats = !engineState->showPerformanceStats;
    }

    ImGui::PushItemWidth(ImGui::GetFontSize());
    
    const char* buttonText = s_capturingPaused ? " Resume Recording " : " Pause Recording ";
    if (ImGui::Button(buttonText))
        s_capturingPaused = !s_capturingPaused;

    ImGui::Separator();

    ImGui::Text("FPS: %i", fps);
    ImGui::Spacing();

    ImGui::Text("FrameTimes (previous %zu frames)", FRAME_HISTORY_COUNT);
    ImGui::Spacing();

    ImGui::Text("This Frame: %.0fms", s_frameTimeHistory[s_currentIndex]);
    ImGui::Spacing();
    
    ImGui::Text("Average: %.1fms", average);
    ImGui::Spacing();

    ImGui::Text("Max: %.0fms", max);
    ImGui::Spacing();

    ImGui::Text("Min: %.0fms", min);
    ImGui::Spacing();

    float chartMax = max > 20 ? max * 1.1 : 20;

    ImGui::PlotLines("##Lines", s_frameTimeHistory, IM_ARRAYSIZE(s_frameTimeHistory),
            s_currentIndex, nullptr, 0.0f, chartMax, ImVec2(-1.f, 200.f));

    ImGui::PopItemWidth();
    ImGui::End();
}

void Velox::DrawMemoryUsageStats() {
    ImGuiWindowFlags flags = 0;
    
    ImGuiInputTextFlags inputTextFlags = 0;
    inputTextFlags |= ImGuiInputTextFlags_EnterReturnsTrue;
    inputTextFlags |= ImGuiInputTextFlags_CallbackCharFilter; 

    ImVec2 windowSize = { 500, 500 };

    // Don't change size of window, just position; To avoid resizing elements.
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(windowSize,  ImGuiCond_FirstUseEver);

    int valueSpacing = 300;

    bool shouldCloseMemoryStats = true;
    ImGui::Begin("Memory Stats", &shouldCloseMemoryStats, flags);
    if (!shouldCloseMemoryStats)
    {
        Velox::EngineState* engineState = Velox::GetEngineState();
        engineState->showMemoryUsageStats = !engineState->showMemoryUsageStats;
    }

    ImGui::PushItemWidth(ImGui::GetFontSize());
    
    ImGui::Separator();

    size_t used, capacity;
    Velox::GetAssetMemoryUsage(&used, &capacity);

    float percentage = (static_cast<float>(used) / static_cast<float>(capacity));

    ImGui::Text("Bytes used / allocated");
    ImGui::Text("Assets: %zu / %zu", used, capacity);
    ImGui::Spacing();

    ImGui::Text("percentage:");

    char buf[64];
    sprintf(buf, "%c%.1f", '%', percentage * 100);
    ImGui::ProgressBar(percentage, ImVec2(-1.f, 0.f), buf);
    ImGui::Spacing();
    
    ImGui::PopItemWidth();
    ImGui::End();
}

static Velox::Arena s_data(100000);

// resolution
static std::vector<SDL_DisplayMode> s_displayModes;
static bool s_updateDisplayModes = true;
static std::vector<char*> s_displayModeLabels;
static int s_selectedDisplayMode = -1;

// vsync
static const char* s_vsyncModes[] = { "Adaptive", "Off", "On" }; // Correspond to (actual + 1).
static i32 s_selectedVsync = 1;

void SaveSettings()
{

}

void ApplySettings()
{

}

void Velox::DrawSettings() {
    if (s_updateDisplayModes)
    {
        int displayModeCount;
        SDL_DisplayMode** displayModes =
            SDL_GetFullscreenDisplayModes(SDL_GetDisplayForWindow(Velox::GetWindow()), &displayModeCount);

        if (displayModes == nullptr)
            printf("ERROR: No display modes? %s\n", SDL_GetError());

        ivec2 currentWindowSize = Velox::GetWindowSize();

        s_displayModes.resize(displayModeCount);
        s_displayModeLabels.resize(displayModeCount);

        for (int i = 0; i < displayModeCount; i++)
        {
            // Copy mode value from pointer
            s_displayModes[i] = *displayModes[i];

            // Create human-readable label
            size_t maxLabelSize = 20;
            s_displayModeLabels[i] = s_data.Alloc<char>(maxLabelSize);
            SDL_snprintf(s_displayModeLabels[i], maxLabelSize, "%i x %i @ %.0fHz",
                s_displayModes[i].w, s_displayModes[i].h, s_displayModes[i].refresh_rate);

            if (s_selectedDisplayMode < 0)
            {
                if (currentWindowSize.x == s_displayModes[i].w &&
                    currentWindowSize.y == s_displayModes[i].h)
                    s_selectedDisplayMode = i;
            }
        }

        SDL_free(displayModes);

        //for (int i = 0; i < displayModeCount; i++)
        //{
        //    printf("%s\n", s_displayModeLabels[i]);
        //}

        s_selectedVsync = Velox::GetVsyncMode() + 1;
        s_updateDisplayModes = false;
    }

    ImGuiWindowFlags flags = 0;
    
    ImGuiInputTextFlags inputTextFlags = 0;
    inputTextFlags |= ImGuiInputTextFlags_EnterReturnsTrue;
    inputTextFlags |= ImGuiInputTextFlags_CallbackCharFilter; 

    ImVec2 windowSize = { 600, 700 };

    // Don't change size of window, just position; To avoid resizing elements.
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(windowSize,  ImGuiCond_FirstUseEver);

    int valueSpacing = 300;

    bool shouldCloseSettings = true;
    ImGui::Begin("Settings", &shouldCloseSettings, flags);
    if (!shouldCloseSettings)
    {
        Velox::EngineState* engineState = Velox::GetEngineState();
        engineState->showSettings = !engineState->showSettings;
    }

    ImGui::PushItemWidth(ImGui::GetFontSize());

    if (ImGui::Button("Save"))
    {
        printf("I do nothing atm\n");
        ApplySettings();
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Display"))
    {
        ImGui::Spacing();

        ImGui::PushItemWidth(ImGui::GetFontSize() * 13);
        ImGui::Text("Resolution: ");
        ImGui::SameLine();

        if (ImGui::BeginCombo("##resolution combo", s_displayModeLabels[s_selectedDisplayMode], flags))
        {
            for (int n = 0; n < s_displayModes.size(); n++)
            {
                const bool isSelected = (s_selectedDisplayMode == n);
                if (ImGui::Selectable(s_displayModeLabels[n], isSelected))
                    s_selectedDisplayMode = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        ImGui::Text("Vsync: ");
        ImGui::SameLine();

        if (ImGui::BeginCombo("##vsync combo", s_vsyncModes[s_selectedVsync], flags))
        {
            for (int n = 0; n < 3; n++)
            {
                const bool isSelected = (s_selectedVsync == n);
                if (ImGui::Selectable(s_vsyncModes[n], isSelected))
                    s_selectedVsync = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        ImGui::PopItemWidth();
    }


    ImGui::PopItemWidth();
    ImGui::End();
}

void Velox::UpdateFrameHistory()
{
    // GM: Wrap around to start.
    s_currentIndex -= 1;
    if (s_currentIndex < 0)
    {
        s_currentIndex = FRAME_HISTORY_COUNT;
    }

    s_frameTimeHistory[s_currentIndex] = Velox::DeltaTimeMS();
}
