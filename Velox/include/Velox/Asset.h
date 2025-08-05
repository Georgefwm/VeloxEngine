#pragma once

#include <Velox.h>

#include "Rendering/Renderer.h"
#include <unordered_map>


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

struct VELOX_API Font {
    char* name;
    std::vector<msdf_atlas::GlyphGeometry> glyphs;
    msdf_atlas::FontGeometry fontGeometry;
    ivec2 atlasResolution;
    Velox::Texture* texture;
};

struct VELOX_API AssetManager {
    // Textures are stored in the renderer
    std::unordered_map<const char*, Velox::Texture> textureMap = {};
    std::unordered_map<const char*, Velox::ShaderProgram> shaderProgramMap = {};
    std::unordered_map<const char*, Velox::Font> fontMap = {};

    Velox::Texture* loadTexture(const char* filepath);
    Velox::Texture* getTexture(const char* filepath);

    Velox::ShaderProgram* loadShaderProgram(const char* vertFilepath, const char* fragFilepath, const char* name);
    Velox::ShaderProgram* getShaderProgram(const char* name);
    Velox::ShaderProgram* reloadShaderProgram(const char* name);

    Velox::Font* loadFont(const char* filepath);
    Velox::Font* getFontRef(const char* filepath);

    void deInit();
};

VELOX_API AssetManager* getAssetManager();

void initAssets();

void deInitAssets();

VELOX_API void getAssetMemoryUsage(size_t* used, size_t* capacity);

}
