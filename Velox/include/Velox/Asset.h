#pragma once

#include "Rendering/Renderer.h"
#include <unordered_map>

#include <msdf-atlas-gen/msdf-atlas-gen.h>

namespace Velox {

enum AssetState {
    Loading,    // Loading file.
    Uploading,  // Waiting for copy pass to gpu (if needed).
    Ready,      // Safe to use.
};

// struct Texture {
//     AssetState state = Loading;
//     SDL_GPUTexture* texture;
//     size_t sizeBytes = 0;
// };

struct Font {
    char* name;
    std::vector<msdf_atlas::GlyphGeometry> glyphs;
    msdf_atlas::FontGeometry fontGeometry;
    ivec2 atlasResolution;
    unsigned int textureId;
};

struct AssetManager {
    // Textures are stored in the renderer
    std::unordered_map<const char*, Velox::Texture> textureMap = {};
    std::unordered_map<const char*, Velox::ShaderProgram> shaderProgramMap = {};
    std::unordered_map<const char*, Velox::Font> fontMap = {};

    // Get texture, load if it hasn't been yet.
    unsigned int LoadTexture(const char* filepath);
    // Get texture, dont try to load if it hasn't been yet.
    unsigned int GetTextureID(const char* filepath);

    unsigned int LoadShaderProgram(const char* vertFilepath, const char* fragFilepath, const char* name);
    unsigned int GetShaderProgramID(const char* name);

    Velox::Font* LoadFont(const char* filepath);
    Velox::Font* GetFontRef(const char* filepath);

    void DeInit();
};

AssetManager* GetAssetManager();

void InitAssets();

void DeInitAssets();

void GetAssetMemoryUsage(size_t* used, size_t* capacity);

}
