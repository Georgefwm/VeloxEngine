#pragma once

#include <spdlog/spdlog.h>

namespace Velox {

struct Log {
    spdlog::logger logger;
};

void InitLog();

spdlog::logger* GetLogger();

}

#define LOG_TRACE(...)    Velox::GetLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)    Velox::GetLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...)     Velox::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)     Velox::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    Velox::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) Velox::GetLogger()->critical(__VA_ARGS__)

