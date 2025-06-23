#include "Console.h"
#include <PCH.h>

#include "ConsoleCommands.h"
#include "Util.h"
#include "Renderer.h"

#include <glm/common.hpp>  // clamp()
#include <imgui.h>
#include <imgui_internal.h>

#include <sstream>

static Velox::Console g_console {};

Velox::Console* Velox::GetConsole() { return &g_console; }

void Velox::InitConsole()
{
    Velox::RegisterDefaultCommands();
}

void Velox::Console::RegisterCommand(const std::string& name, 
        std::function<void(std::string&, const std::vector<std::string>&)> func)
{
    // Check for command name collisions.
    bool collisionFound = false;
    if (commands.find(name) != commands.end())
    {
        collisionFound = true;
    }

    if (collisionFound)
    {
        printf("Command \"%s\" is already registered, skipping\n", name.c_str());
        return;
    }

    commands[name] = std::move(func);
}

bool Velox::Console::ExecuteCommand(const std::string& inputLine)
{
    std::stringstream stream(inputLine);
    std::string commandName;
    stream >> commandName;

    Velox::ConsoleRecord record;
    record.command = inputLine;

    if (commands.find(commandName) == commands.end())
    {
        record.response = "Unknown command";
        history.push_back(record);
        return false;
    }

    std::vector<std::string> args;
    std::string arg;
    while (stream >> arg) {
        args.push_back(arg);
    }

    // Run command.
    commands.at(commandName)(record.response, args);

    history.push_back(record);

    return true;
}

// TODO: Tab autocomplete on available commands.
std::vector<std::string> GetSuggestions(const std::string& prefix) 
{
    return { "" };
}

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

    // history entriesfor
    for (auto record = g_console.history.rbegin(); record != g_console.history.rend(); record++)
    {                    
        ImGui::Text("> %s", record->command.c_str());
        ImGui::Text("%s", record->response.c_str());
    }
    
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

    if (ImGui::InputTextWithHint("##CmdInputLine", "Use \"list\" to view all commands",
        g_console.commandInput, INPUT_BUFFER_SIZE, inputTextFlags, Velox::ConsoleKeyFilterCallback, NULL))
    {
        std::string command { g_console.commandInput };

        g_console.ExecuteCommand(command);

        // Clear input buffer.
        g_console.commandInput[0] = '\0';

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

