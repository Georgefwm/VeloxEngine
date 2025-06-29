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

Velox::Config* Velox::GetConfig() { return &s_config; }

bool TomlToConfig(toml::table* table, Velox::Config* config)
{
    if (table == nullptr)
    {
        printf("ERROR: Config table is null\n");
        return false;
    }

    config->windowWidth  = table->at_path("rendering.window_width" ).value_or(config->windowWidth);
    config->windowHeight = table->at_path("rendering.window_height").value_or(config->windowHeight);
    config->vsyncMode    = table->at_path("rendering.vsync_mode"   ).value_or(config->vsyncMode);

    return true;
}

bool ConfigToToml(Velox::Config* config, toml::table* table)
{
    if (table == nullptr)
    {
        printf("ERROR: Config table is null\n");
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
bool WriteToFile(const char* filepath, toml::table* table)
{
    if (table == nullptr)
    {
        printf("ERROR: Failed to write to file, table is nullptr\n");
        return false;
    }

    std::ofstream file;

    // Open file at start, discard old data.
    file.open(filepath, std::ofstream::trunc);

    if (!file.is_open())
    {
        printf("Failed to write new default config file: %s\n", filepath);
        return false;
    }

    file << *table;
    file.close();

    return true;
}

void Velox::SaveUserConfig()
{
    toml::table newUserTable {};
    ConfigToToml(&s_config, &newUserTable);
    WriteToFile(s_userConfigPath, &newUserTable);
}

void CreateDefaultConfig()
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
    WriteToFile(s_defaultConfigPath, &s_defaultTable);
}



void Velox::InitConfig(bool* userConfigExists)
{
    const size_t pathSize = 1024;

    s_defaultConfigPath = s_data.Alloc<char>(pathSize);
    s_userConfigPath    = s_data.Alloc<char>(pathSize);

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
            CreateDefaultConfig();

        if (userExists)
            s_userTable = toml::parse_file(s_userConfigPath);

    }
    catch (const toml::parse_error& err)
    {
        printf("Failed to parse toml: %s\n", err.description().data());
        return;
    }

    if (userExists)
        TomlToConfig(&s_userTable, &s_config);        
    else
        TomlToConfig(&s_defaultTable, &s_config);        

}


