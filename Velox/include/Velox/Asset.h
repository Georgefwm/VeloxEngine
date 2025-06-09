#pragma once

#include <unordered_map>
#include "Arena.h"
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
    // Textures are stored in the renderer
    std::unordered_map<const char*, int> textureMap = {
        { "default_texture", 0 },
    };

    // Returns default texture on failure.
    int LoadTexture(const char* filepath);

    void DeInit();
};

AssetManager* GetAssetManager();

void InitAssets();

void GetAssetMemoryUsage(size_t* used, size_t* capacity);

}
