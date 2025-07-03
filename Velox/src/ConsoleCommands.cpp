#include "ConsoleCommands.h"
#include <PCH.h>

#include "Asset.h"
#include "Console.h"
#include "Core.h"
#include <sstream>

void Velox::registerDefaultCommands()
{
    Velox::Console* console = getConsole();

    // List all available commands.
    console->registerCommand("list", &Velox::listCommands);

    // Toggle performance stats window.
    console->registerCommand("fps", &Velox::fpsCommand);

    // Toggle memory stats window.
    console->registerCommand("mem", &Velox::memoryCommand);

    // Toggle settings window.
    console->registerCommand("settings", &Velox::settingsCommand);

    // Reload shader.
    console->registerCommand("reloadshader", &Velox::reloadShaderCommand);

    // Toggle entity info.
    console->registerCommand("entity", &Velox::entityCommand);

    // Request quit.
    console->registerCommand("quit", &Velox::quitCommand);
}

void Velox::listCommands(std::string& response, const std::vector<std::string> args)
{
    Velox::Console* console = Velox::getConsole();

    std::stringstream string;
    string << "Registered commands:\n\n";

    for (const auto& pair : console->commands)
    {
        string << pair.first << "\n";
    }

    string << "\n";

    response = string.str();
}

void Velox::fpsCommand(std::string& response, const std::vector<std::string> args)
{
    Velox::EngineState* engineState = Velox::getEngineState();
    engineState->showPerformanceStats = !engineState->showPerformanceStats;
}

void Velox::memoryCommand(std::string& response, const std::vector<std::string> args)
{
    Velox::EngineState* engineState = Velox::getEngineState();
    engineState->showMemoryUsageStats = !engineState->showMemoryUsageStats;
}

void Velox::settingsCommand(std::string& response, const std::vector<std::string> args)
{
    Velox::EngineState* engineState = Velox::getEngineState();
    engineState->showSettings = !engineState->showSettings;
}

void Velox::reloadShaderCommand(std::string& response, const std::vector<std::string> args)
{
    if (args.size() < 1)
        response += "No shader name given\n";

    Velox::AssetManager* assetManager = Velox::getAssetManager();
    for (auto& shaderName : args)
    {
        Velox::ShaderProgram* shader = assetManager->reloadShaderProgram(shaderName.c_str());
        if (shader == nullptr)
        {
            response += fmt::format("Failed to reload shader '{}'\n", shaderName);
            continue;
        }

        response += fmt::format("Reloaded shader '{}'\n", shaderName); 
    }
}

void Velox::entityCommand(std::string& response, const std::vector<std::string> args)
{

    Velox::EngineState* engineState = Velox::getEngineState();
    engineState->showEntityInfo = !engineState->showEntityInfo;
}

void Velox::quitCommand(std::string& response, const std::vector<std::string> args)
{
    response = "Quitting...\n";
    Velox::quit();
}
