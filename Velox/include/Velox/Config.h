#pragma once

namespace Velox {

struct Config {
    int windowWidth = 1920;
    int windowHeight = 1080;
    int vsyncMode = 1;
};

Config* GetConfig();

void SaveUserConfig();

void InitConfig(bool* userConfigExists);

}
