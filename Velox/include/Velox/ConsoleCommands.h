#pragma once

#include "Velox.h"

#include <string>
#include <vector>

namespace Velox {

void registerDefaultCommands();

void listCommands(std::string& response, const std::vector<std::string> args);
void fpsCommand(std::string& response, const std::vector<std::string> args);
void memoryCommand(std::string& response, const std::vector<std::string> args);
void settingsCommand(std::string& response, const std::vector<std::string> args);
void reloadShaderCommand(std::string& response, const std::vector<std::string> args);
void entityCommand(std::string& response, const std::vector<std::string> args);
void colliderCommand(std::string& response, const std::vector<std::string> args);
void quitCommand(std::string& response, const std::vector<std::string> args);

}
