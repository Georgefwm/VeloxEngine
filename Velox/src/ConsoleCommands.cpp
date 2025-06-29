#include "ConsoleCommands.h"
#include <PCH.h>

#include "Asset.h"
#include "Console.h"
#include "Velox.h"
#include <sstream>

void Velox::RegisterDefaultCommands()
{
    Velox::Console* console = GetConsole();

    // List all available commands.
    console->RegisterCommand("list", &Velox::ListCommands);

    // Toggle performance stats window.
    console->RegisterCommand("fps", &Velox::FpsCommand);

    // Toggle memory stats window.
    console->RegisterCommand("mem", &Velox::MemoryCommand);

    // Toggle settings window.
    console->RegisterCommand("settings", &Velox::SettingsCommand);

    // Reload shader.
    console->RegisterCommand("reloadshader", &Velox::ReloadShaderCommand);

    // Request quit.
    console->RegisterCommand("quit", &Velox::QuitCommand);
}

void Velox::ListCommands(std::string& response, const std::vector<std::string> args)
{
    Velox::Console* console = Velox::GetConsole();

    std::stringstream string;
    string << "Registered commands:\n\n";

    for (const auto& pair : console->commands)
    {
        string << pair.first << "\n";
    }

    string << "\n";

    response = string.str();
}

void Velox::FpsCommand(std::string& response, const std::vector<std::string> args)
{
    Velox::EngineState* engineState = Velox::GetEngineState();
    engineState->showPerformanceStats = !engineState->showPerformanceStats;
}

void Velox::MemoryCommand(std::string& response, const std::vector<std::string> args)
{
    Velox::EngineState* engineState = Velox::GetEngineState();
    engineState->showMemoryUsageStats = !engineState->showMemoryUsageStats;
}

void Velox::SettingsCommand(std::string& response, const std::vector<std::string> args)
{
    Velox::EngineState* engineState = Velox::GetEngineState();
    engineState->showSettings = !engineState->showSettings;
}

void Velox::ReloadShaderCommand(std::string& response, const std::vector<std::string> args)
{
    if (args.size() < 1)
        response += "No shader name given\n";

    Velox::AssetManager* assetManager = Velox::GetAssetManager();
    for (auto& shaderName : args)
    {
        Velox::ShaderProgram* shader = assetManager->ReloadShaderProgram(shaderName.c_str());
        if (shader == nullptr)
        {
            response += fmt::format("Failed to reload shader '{}'\n", shaderName);
            continue;
        }

        response += fmt::format("Reloaded shader '{}'\n", shaderName); 
    }
}

void Velox::QuitCommand(std::string& response, const std::vector<std::string> args)
{
    response = "Quitting...\n";
    Velox::Quit();
}
