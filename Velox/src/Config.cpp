#include "Config.h"
#include <PCH.h>

#include "Arena.h"
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_video.h>
#include <fstream>
#include <toml++/toml.hpp>

char* s_defaultConfigPath;
char* s_userConfigPath;

Velox::Arena s_data(100000);

toml::table s_defaultTable;
toml::table s_userTable;

Velox::Config s_config;

Velox::Config* Velox::getConfig() { return &s_config; }

bool tomlToConfig(toml::table* table, Velox::Config* config)
{
    if (table == nullptr)
    {
        LOG_ERROR("Table is null");
        return false;
    }

    config->windowWidth  = table->at_path("rendering.window_width" ).value_or(config->windowWidth);
    config->windowHeight = table->at_path("rendering.window_height").value_or(config->windowHeight);
    config->vsyncMode    = table->at_path("rendering.vsync_mode"   ).value_or(config->vsyncMode);

    return true;
}

bool configToToml(Velox::Config* config, toml::table* table)
{
    if (table == nullptr)
    {
        LOG_ERROR("Table is null");
        return false;
    }

    toml::table renderingConfig = toml::table {
        { "window_width",  config->windowWidth  },
        { "window_height", config->windowHeight },
        { "vsync_mode",    config->vsyncMode    },
    };

    *table = toml::table {
        { "rendering", renderingConfig },
    };

    return true;
}

// Overwrites file, remember to include *all* information every write.
bool writeToFile(const char* filepath, toml::table* table)
{
    if (table == nullptr)
    {
        LOG_ERROR("Failed to write to file, table is nullptr");
        return false;
    }

    std::ofstream file;

    // Open file at start, discard old data.
    file.open(filepath, std::ofstream::trunc);

    if (!file.is_open())
    {
        LOG_WARN("Failed to write new default config file: {}", filepath);
        return false;
    }

    file << *table;
    file.close();

    return true;
}

void Velox::saveUserConfig()
{
    toml::table newUserTable {};
    configToToml(&s_config, &newUserTable);
    writeToFile(s_userConfigPath, &newUserTable);
}

void createDefaultConfig()
{
    // Detect native resolution.
    SDL_DisplayID displayID = SDL_GetPrimaryDisplay();
    SDL_Rect displayBounds;
    SDL_GetDisplayBounds(displayID, &displayBounds);

    toml::table renderingConfig = toml::table {
        { "window_width",  displayBounds.w },
        { "window_height", displayBounds.h },
        { "vsync_mode",    1 },
    };

    s_defaultTable = toml::table {
        { "rendering", renderingConfig },
    };

    // Write to file.
    writeToFile(s_defaultConfigPath, &s_defaultTable);
}

void Velox::initConfig(bool* userConfigExists)
{
    const size_t pathSize = 1024;

    s_defaultConfigPath = s_data.alloc<char>(pathSize);
    s_userConfigPath    = s_data.alloc<char>(pathSize);

    const char* basePath = SDL_GetBasePath();

    SDL_strlcpy(s_defaultConfigPath, basePath, pathSize);
    SDL_strlcpy(s_userConfigPath,    basePath, pathSize);

    SDL_strlcat(s_defaultConfigPath, "default.toml", pathSize);
    SDL_strlcat(s_userConfigPath,    "user.toml",    pathSize);

    bool defaultExists = SDL_GetPathInfo(s_defaultConfigPath, nullptr);
    bool userExists    = SDL_GetPathInfo(s_userConfigPath,    nullptr);

    if (userConfigExists != nullptr)
        *userConfigExists = userExists;

    try
    {
        if (defaultExists)
            s_defaultTable = toml::parse_file(s_defaultConfigPath);
        else
            createDefaultConfig();

        if (userExists)
            s_userTable = toml::parse_file(s_userConfigPath);

    }
    catch (const toml::parse_error& err)
    {
        LOG_WARN("Failed to parse toml: {}", err.description().data()); 
        return;
    }

    if (userExists)
        tomlToConfig(&s_userTable, &s_config);        
    else
        tomlToConfig(&s_defaultTable, &s_config);        

}

