#pragma once

#include "Velox.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

struct ImGuiInputTextCallbackData; 

constexpr size_t INPUT_BUFFER_SIZE = 256; 

namespace Velox {

struct ConsoleRecord {
    std::string command;
    std::string response;
};

struct Console {
    float maxHeight            = 400.0;
    float currentHeight        = 0.0;
    float openSpeed            = 3000.0;
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

int consoleKeyFilterCallback(ImGuiInputTextCallbackData* data);

}
