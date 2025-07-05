#pragma once

#include <Velox.h>

namespace Velox {

void initLog();

VELOX_API spdlog::logger* getLogger();

}

#define LOG_TRACE(...)    Velox::getLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)    Velox::getLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...)     Velox::getLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)     Velox::getLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    Velox::getLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) Velox::getLogger()->critical(__VA_ARGS__)

