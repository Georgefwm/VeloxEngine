#include "ConsoleCommands.h"

#include "Console.h"
#include "Velox.h"
#include <sstream>

void Velox::RegisterDefaultCommands()
{
    Velox::Console* console = GetConsole();

    console->RegisterCommand("list", &Velox::ListCommands); 
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

void Velox::QuitCommand(std::string& response, const std::vector<std::string> args)
{
    response = "Quitting...";
    Velox::Quit();
}
