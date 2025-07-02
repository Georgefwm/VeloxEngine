#pragma once

#include "Velox.h"

namespace Velox {

struct VELOX_API Config {
    int windowWidth = 1920;
    int windowHeight = 1080;
    int vsyncMode = 1;
};

VELOX_API Config* getConfig();

VELOX_API void saveUserConfig();

void initConfig(bool* userConfigExists);

}
