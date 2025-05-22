#include "Console.h"

#include "Util.h"
#include "Renderer.h"

#include <glm/common.hpp>  // clamp()
#include <imgui.h>
#include <imgui_internal.h>

static Velox::Console g_console {};

void Velox::ToggleConsole()
{
    g_console.shouldBeOpen = !g_console.shouldBeOpen;
}

void Velox::DrawConsole()
{
    if (!g_console.shouldBeOpen && g_console.currentHeight <= 0.0)
        return;

    float deltaHeight = g_console.openSpeed * Velox::DeltaTime();

    // Should console be opening or closing.
    deltaHeight *= g_console.shouldBeOpen * 2 - 1;

    // Clamp to stop moving further than we want.
    g_console.currentHeight = glm::clamp(g_console.currentHeight + deltaHeight, 0.0f, g_console.maxHeight);

    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoNav;
    flags |= ImGuiWindowFlags_NoDecoration;
    
    ImGuiInputTextFlags inputTextFlags = 0;
    inputTextFlags |= ImGuiInputTextFlags_EnterReturnsTrue;
    inputTextFlags |= ImGuiInputTextFlags_CallbackCharFilter; 

    // Don't change size of window, just position; To avoid resizing elements.
    ImGui::SetNextWindowPos(ImVec2(0, 0 - g_console.maxHeight + g_console.currentHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(Velox::GetWindowSize().x, g_console.maxHeight),   ImGuiCond_Always);

    int varSpacing = 100;

    ImGui::Begin("Console Window", NULL, flags);
    ImGui::PushItemWidth(ImGui::GetFontSize());
    // ImGui::PushItemWidth(-ImGui::GetStyle().ItemSpacing.x * 7);
    const float footerHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

    ImGui::Text("Console");
    ImGui::Separator();

    ImGui::BeginChild("ScrollRegion##", ImVec2(0, -footerHeightToReserve), false, 0);

    ImGui::PushTextWrapPos();

    // history entries
    // for (int index = 0; index < g_console.history.size(); index++)
    // {                    
    //     ImGui::Text("> %s", g_console.history[index].command.c_str());
    // 
    //     for (int responseIndex = 0; responseIndex < g_console.history[index].response.size(); responseIndex++)
    //     {
    //         ImGui::Text("%s", g_console.history[index].response[responseIndex].c_str());
    //     }
    // }
    
    ImGui::PopTextWrapPos();

    // Auto-scroll logs.
    if (g_console.shouldScrollToBottom && (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
        ImGui::SetScrollHereY(1.0f);

    g_console.shouldScrollToBottom = false;

    ImGui::EndChild();

    ImGui::Separator();

    ImGui::PushItemWidth(-ImGui::GetStyle().ItemSpacing.x * 8);

    if (g_console.bufferEditedManually)
    {
        // Tell ImGui to reload the buffer when we strcpy to it manually.
        if (ImGuiInputTextState* state = ImGui::GetInputTextState(ImGui::GetID("##CmdInputLine")))
            state->ReloadUserBufAndMoveToEnd();

        g_console.bufferEditedManually = false;
    }

    if (ImGui::InputText("##CmdInputLine", g_console.commandInput, INPUT_BUFFER_SIZE,
                inputTextFlags, Velox::ConsoleKeyFilterCallback, NULL))
    {
        // g_console.SubmitCommand();

        // Scroll to bottom when command is entered.
        g_console.shouldScrollToBottom = true;
    }

    // Always keep focus while console is open.
    if (g_console.shouldBeOpen) 
        ImGui::SetKeyboardFocusHere(-1);

    // Don't capture input while console is closing.
    else
        ImGui::SetWindowFocus(NULL);

    ImGui::PopItemWidth();
    ImGui::End();
}

int static Velox::ConsoleKeyFilterCallback(ImGuiInputTextCallbackData* data)
{
    switch (data->EventChar)
    {
        // Might not need to set this, but we might want it later.
        case 96: return 1;  // ` (grave char)
        default: return 0;
    }
}

