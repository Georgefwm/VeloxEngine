#include "Debug.h"

#include "Util.h"
#include "imgui.h"

constexpr size_t FRAME_HISTORY_COUNT = 100;

static float g_frameTimeHistory[FRAME_HISTORY_COUNT];
static int   g_currentIndex    = 0;
static bool  g_capturingPaused = false;

void Velox::DrawPerformanceStats()
{
    if (!g_capturingPaused)
        Velox::UpdateFrameHistory();
    
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

    ImGui::Begin("Perormance Stats", NULL, flags);
    ImGui::PushItemWidth(ImGui::GetFontSize());
    
    const char* buttonText = g_capturingPaused ? " Resume Recording " : " Pause Recording ";
    if (ImGui::Button(buttonText))
        g_capturingPaused = !g_capturingPaused;

    ImGui::Separator();

    ImGui::Spacing();
    ImGui::Text("FrameTime (ms): %.0f", g_frameTimeHistory[g_currentIndex]);

    ImGui::Spacing();
    ImGui::Text("Chart: Previous %zu frames", FRAME_HISTORY_COUNT);

    float max, sum;
    for (int i = 0; i < FRAME_HISTORY_COUNT; i++)
    {
        float value = g_frameTimeHistory[i];

        if (value <= 0)
            continue;

        if (value > max)
            max = value;

        sum += value;
    }

    float average = sum / FRAME_HISTORY_COUNT;
    float chartMax = max > 20 ? max * 1.1 : 20;

    ImGui::PlotLines("##Lines", g_frameTimeHistory, IM_ARRAYSIZE(g_frameTimeHistory), g_currentIndex, nullptr,
            0.0f, chartMax, ImVec2(windowSize.x -20, 100.f));

    ImGui::PopItemWidth();
    ImGui::End();
}

void Velox::UpdateFrameHistory()
{
    // GM: Wrap around to start.
    g_currentIndex = (g_currentIndex + 1) % FRAME_HISTORY_COUNT;

    g_frameTimeHistory[g_currentIndex] = Velox::DeltaTimeMS();
}
