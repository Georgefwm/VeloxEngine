#include "Log.h"
#include <PCH.h>

#include <SDL3/SDL_filesystem.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

static spdlog::logger logger("pre-init");

void Velox::initLog()
{
    std::string logFileOutput = fmt::format("{}logs/logs.txt", SDL_GetBasePath());

    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    consoleSink->set_level(spdlog::level::trace);
    consoleSink->set_pattern("[%^%l%$] %v");

    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFileOutput, true);
    fileSink->set_level(spdlog::level::trace);
    fileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

    logger = spdlog::logger("multi_sink_logger", { consoleSink, fileSink });
    logger.set_level(spdlog::level::trace);
}

spdlog::logger* Velox::getLogger()
{
    return &logger;
}

