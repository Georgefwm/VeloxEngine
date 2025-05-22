#pragma once

#include <queue>
#include <vector>

struct ImGuiInputTextCallbackData; 

constexpr size_t INPUT_BUFFER_SIZE = 256; 

namespace Velox {

struct Command {
    char command;
    char* args;
};

struct Response {
    char* response;
};

struct ConsoleRecord {
    Command command;
    Response response;
};

struct Console {
    bool  shouldBeOpen  = false;
    float maxHeight     = 400.0;
    float currentHeight = 0.0;
    float openSpeed     = 3000.0;

    bool shouldScrollToBottom = false;

    std::vector<ConsoleRecord> history;
    std::queue<ConsoleRecord> commandQueue;
    int historyIndex = -1;

    char commandInput[INPUT_BUFFER_SIZE];
    bool bufferEditedManually = false;
};

void ToggleConsole();

void DrawConsole();

int ConsoleKeyFilterCallback(ImGuiInputTextCallbackData* data);

}
