#include "Debug.h"

#include "Asset.h"
#include "SDL3/SDL_timer.h"
#include "Util.h"
#include "imgui.h"

constexpr size_t FRAME_HISTORY_COUNT = 1000;

static float g_frameTimeHistory[FRAME_HISTORY_COUNT];
static int  g_currentIndex    = FRAME_HISTORY_COUNT;
static bool g_capturingPaused = false;

void Velox::DrawPerformanceStats()
{
    if (!g_capturingPaused)
        Velox::UpdateFrameHistory();
    
    int count = 0;
    int validCount = 0;
    float max = 0;
    float min = 10000;
    float sum = 0;
    float sumToSecond = 0;
    int i = g_currentIndex; // Reverse order indexing.
    int fps = 0;
    while (count <= FRAME_HISTORY_COUNT)
    {
        count += 1;

        if (i >= FRAME_HISTORY_COUNT)
            i = 0;

        float value = g_frameTimeHistory[i];

        i = (i + 1) % FRAME_HISTORY_COUNT;

        if (value <= 0)
            continue;

        validCount += 1;

        if (sumToSecond <= SDL_MS_PER_SECOND)
        {
            sumToSecond += value;
            fps++;
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

    ImGui::Begin("Perormance Stats", NULL, flags);
    ImGui::PushItemWidth(ImGui::GetFontSize());
    
    const char* buttonText = g_capturingPaused ? " Resume Recording " : " Pause Recording ";
    if (ImGui::Button(buttonText))
        g_capturingPaused = !g_capturingPaused;

    ImGui::Separator();

    ImGui::Text("FPS: %i", fps);
    ImGui::Spacing();

    ImGui::Text("FrameTimes (previous %zu frames)", FRAME_HISTORY_COUNT);
    ImGui::Spacing();

    ImGui::Text("This Frame: %.0fms", g_frameTimeHistory[g_currentIndex]);
    ImGui::Spacing();
    
    ImGui::Text("Average: %.1fms", average);
    ImGui::Spacing();

    ImGui::Text("Max: %.0fms", max);
    ImGui::Spacing();

    ImGui::Text("Min: %.0fms", min);
    ImGui::Spacing();

    float chartMax = max > 20 ? max * 1.1 : 20;

    ImGui::PlotLines("##Lines", g_frameTimeHistory, IM_ARRAYSIZE(g_frameTimeHistory),
            g_currentIndex, nullptr, 0.0f, chartMax, ImVec2(-1.f, 200.f));

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

    ImGui::Begin("Memory Stats", NULL, flags);
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

void Velox::UpdateFrameHistory()
{
    // GM: Wrap around to start.
    g_currentIndex -= 1;
    if (g_currentIndex < 0)
    {
        g_currentIndex = FRAME_HISTORY_COUNT;
    }

    g_frameTimeHistory[g_currentIndex] = Velox::DeltaTimeMS();
}
