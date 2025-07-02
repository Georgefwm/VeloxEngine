#include "Debug.h"
#include <PCH.h>
#include <SDL3/SDL_video.h>

#include "Arena.h"
#include "Asset.h"
#include "Config.h"
#include "Rendering/Renderer.h"
#include "Text.h"
#include "Timing.h"
#include "Util.h"
#include "Core.h"
#include "imgui.h"

constexpr size_t FRAME_HISTORY_COUNT = 1000;

static float s_frameTimeHistory[FRAME_HISTORY_COUNT];
static int  s_currentIndex    = FRAME_HISTORY_COUNT;
static bool s_capturingPaused = false;

void Velox::drawPerformanceStats()
{
    if (!s_capturingPaused)
        Velox::updateFrameHistory();
    
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
        Velox::EngineState* engineState = Velox::getEngineState();
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

void Velox::drawMemoryUsageStats() {
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
        Velox::EngineState* engineState = Velox::getEngineState();
        engineState->showMemoryUsageStats = !engineState->showMemoryUsageStats;
    }

    ImGui::PushItemWidth(ImGui::GetFontSize());
    
    ImGui::Separator();

    size_t used, capacity;
    Velox::getAssetMemoryUsage(&used, &capacity);

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

void saveSettings()
{
    Velox::saveUserConfig();
}

void applySettings()
{
    LOG_INFO("I do nothing atm");
}

void Velox::drawSettings() {
    if (s_updateDisplayModes)
    {
        int displayModeCount;
        SDL_DisplayMode** displayModes =
            SDL_GetFullscreenDisplayModes(SDL_GetDisplayForWindow(Velox::GetWindow()), &displayModeCount);

        if (displayModes == nullptr)
            LOG_ERROR("No display modes found");

        ivec2 currentWindowSize = Velox::getWindowSize();

        s_displayModes.clear();
        s_displayModeLabels.clear();

        int insertedIndex = 0;
        for (int i = 0; i < displayModeCount; i++)
        {
            if (s_displayModes.size() > 0)
            {
                // Skip entries with only different refresh rates (only max is ever used).
                if (displayModes[i]->w == s_displayModes.back().w && 
                    displayModes[i]->h == s_displayModes.back().h)
                    continue;
            }

            // Copy mode value from pointer
            s_displayModes.push_back(*displayModes[i]);

            size_t maxLabelSize = 14; // (5 chars) + " x " + (5 chars) + '\0'.
            char* labelPtr  = s_data.alloc<char>(maxLabelSize);
            SDL_snprintf(labelPtr, maxLabelSize, "%i x %i",
                s_displayModes[insertedIndex].w, s_displayModes[insertedIndex].h);

            s_displayModeLabels.push_back(labelPtr);

            if (s_selectedDisplayMode < 0)
            {
                if (currentWindowSize.x == s_displayModes[insertedIndex].w &&
                    currentWindowSize.y == s_displayModes[insertedIndex].h)
                    s_selectedDisplayMode = i;
            }

            insertedIndex += 1;
        }

        SDL_free(displayModes);

        //for (int i = 0; i < displayModeCount; i++)
        //{
        //    printf("%s\n", s_displayModeLabels[i]);
        //}

        s_selectedVsync = Velox::getVsyncMode() + 1;
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
        Velox::EngineState* engineState = Velox::getEngineState();
        engineState->showSettings = !engineState->showSettings;
    }

    ImGui::PushItemWidth(ImGui::GetFontSize());

    if (ImGui::Button("Save"))
    {
        saveSettings();
    }

    ImGui::SameLine();
    if (ImGui::Button("Force Update"))
        s_updateDisplayModes = true;

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Display"))
    {
        ImGui::Spacing();

        // Set to the longest label name (currently Resolution)
        const float alignedElementOffset = ImGui::GetFontSize() * 7;

        ImGui::Text("Resolution:");

        ImGui::SameLine(alignedElementOffset);
        ImGui::PushItemWidth(ImGui::GetFontSize() * 8);
        if (ImGui::BeginCombo("##resolution combo", s_displayModeLabels[s_selectedDisplayMode], flags))
        {
            for (int n = 0; n < s_displayModes.size(); n++)
            {
                const bool isSelected = (s_selectedDisplayMode == n);
                if (ImGui::Selectable(s_displayModeLabels[n], isSelected))
                {
                    // Prevent re-applying same resolution.
                    if (s_selectedDisplayMode == n)
                        continue;

                    Velox::setResolution(ivec2(s_displayModes[n].w, s_displayModes[n].h));
                    s_selectedDisplayMode = n;
                }

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();

        ImGui::Text("Vsync: ");

        ImGui::SameLine(alignedElementOffset);
        ImGui::PushItemWidth(ImGui::GetFontSize() * 7);
        if (ImGui::BeginCombo("##vsync combo", s_vsyncModes[s_selectedVsync], flags))
        {
            // Adaptive vsync is always first item, so skip if not supported.
            int vsyncStartIndex = Velox::isAdaptiveVsyncSupported() ? 0 : 1;

            for (int n = vsyncStartIndex; n < 3; n++)
            {
                const bool isSelected = (s_selectedVsync == n);
                if (ImGui::Selectable(s_vsyncModes[n], isSelected))
                {
                    // Prevent re-applying same resolution.
                    if (s_selectedVsync == n)
                        continue;

                    Velox::setVsyncMode(n - 1);
                    s_selectedVsync = n;
                }

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();

    }

    ImGui::End();
}

static Velox::TextDrawStyle s_editorStyle {};

void Velox::textStyleEditor(Velox::TextDrawStyle* style, bool useCurrentAsBase)
{
    if (style == nullptr)
        return;

    if (useCurrentAsBase)
    {
        *style = *Velox::GetUsingTextStyle();
    }

    ImGuiWindowFlags flags = 0;
    
    ImGuiInputTextFlags inputTextFlags = 0;
    inputTextFlags |= ImGuiInputTextFlags_EnterReturnsTrue;
    inputTextFlags |= ImGuiInputTextFlags_CallbackCharFilter; 

    ImVec2 windowSize = { 500, 500 };

    // Don't change size of window, just position; To avoid resizing elements.
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(windowSize,  ImGuiCond_FirstUseEver);

    int valueSpacing = 300;

    ImGui::Begin("Text Style Editor", NULL, flags);

    if (ImGui::Button("Reset"))
    {
        s_editorStyle = {};
    }

    ImGui::Separator();

    const ImGuiSliderFlags flags_for_sliders = flags & ~ImGuiSliderFlags_WrapAround;
    ImGui::ColorEdit4("Text Color", (float*)&s_editorStyle.color);
    ImGui::SliderFloat("Font Weight", &s_editorStyle.fontWeightBias, 0.0f, 0.99f, "%.2f", flags_for_sliders);
    ImGui::ColorEdit4("Outline Color", (float*)&s_editorStyle.outlineColor);
    ImGui::SliderFloat("OutLine Width", &s_editorStyle.outlineWidth, 0.0f, 1.99f, "%.2f", flags_for_sliders);
    ImGui::SliderFloat("OutLine Blur", &s_editorStyle.outlineBlur, 0.0f, 1.99f, "%.2f", flags_for_sliders);
    
    ImGui::End();

    style->color = s_editorStyle.color;
    style->fontWeightBias = s_editorStyle.fontWeightBias;
    style->outlineColor = s_editorStyle.outlineColor;
    style->outlineWidth = s_editorStyle.outlineWidth;
    style->outlineBlur = s_editorStyle.outlineBlur;
}

void Velox::updateFrameHistory()
{
    // GM: Wrap around to start.
    s_currentIndex -= 1;
    if (s_currentIndex < 0)
    {
        s_currentIndex = FRAME_HISTORY_COUNT;
    }

    s_frameTimeHistory[s_currentIndex] = Velox::getDeltaTime();
}
