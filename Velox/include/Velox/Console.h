#pragma once

#include "Velox.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

struct ImGuiInputTextCallbackData; 
SDL_EVENT_FWD_DECL

constexpr size_t INPUT_BUFFER_SIZE = 256; 

namespace Velox {

struct ConsoleRecord {
    std::string command;
    std::string response;
};

struct Console {
    float maxHeightPercent     = 0.4f;
    float currentHeight        = 0.0f;
    float openSpeed            = 5000.0f;
    bool  shouldBeOpen         = false;
    bool  shouldScrollToBottom = false; // Currently goes to top.

    std::vector<ConsoleRecord> history;
    int historyIndex = -1;

    std::unordered_map<std::string,
        std::function<void(std::string&, const std::vector<std::string>&)>> commands;

    char commandInput[INPUT_BUFFER_SIZE];
    bool bufferEditedManually = false;

    // GM: This function signature looks pretty gross, means it takes a function that
    // looks like this:
    //
    // void FuncName(std::string* response, const std::vector<std::string> args);
    //
    // Usage example:
    // Velox::getConsole()->registerCommand("mycommand", &FuncName);
    //
    void registerCommand(const std::string& name,
            std::function<void(std::string&, const std::vector<std::string>&)> func);

    bool executeCommand(const std::string& inputLine);

    std::vector<std::string> getSuggestions(const std::string& prefix) const;
};

VELOX_API Console* getConsole();

VELOX_API void printToConsole(const std::string& string);

void initConsole();

VELOX_API void toggleConsole();

void drawConsole();

bool consoleEventCallback(SDL_Event& event);

int consoleKeyFilterCallback(ImGuiInputTextCallbackData* data);

}
