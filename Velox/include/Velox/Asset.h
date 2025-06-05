#pragma once

#include <vector>
#include "SDL3/SDL_gpu.h"

namespace Velox {

enum AssetState {
    Loading,    // Loading file.
    Uploading,  // Waiting for copy pass to gpu (if needed).
    Ready,      // Safe to use.
};

struct Texture {
    AssetState state = Loading;
    SDL_GPUTexture* texture;
    size_t sizeBytes = 0;
};

struct AssetManager {
    std::vector<Texture> textures;

    void DeInit();
};

void InitAssets();

void LoadTexture(const char& filepath);

void GetAssetMemoryUsage(size_t* used, size_t* capacity);

}
