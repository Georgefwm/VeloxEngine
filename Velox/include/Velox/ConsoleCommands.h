#pragma once

#include <string>
#include <vector>

namespace Velox {

void RegisterDefaultCommands();

void ListCommands(std::string& response, const std::vector<std::string> args);

void FpsCommand(std::string& response, const std::vector<std::string> args);

void QuitCommand(std::string& response, const std::vector<std::string> args);


}
