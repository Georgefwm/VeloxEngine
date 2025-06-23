#include "Asset.h"
#include <PCH.h>

#include "Arena.h"
#include "Renderer.h"
#include "msdf-atlas-gen/types.h"
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>
#include <glad/gl.h>
#include <msdf-atlas-gen/msdf-atlas-gen.h>

static Velox::Arena g_assetStorage(1024);
static Velox::AssetManager g_assetManager {};
static msdfgen::FreetypeHandle* g_freetype;

unsigned int Velox::AssetManager::LoadTexture(const char* filepath)
{
    for (auto pair : textureMap)
    {
        if (strcmp(pair.first, filepath) == 0)
            return pair.second.id;
    }

    Velox::Arena tempData(2048);
    // Haven't loaded texture before so send load for renderer to deal with.
    Velox::Texture texture = Velox::LoadTexture(filepath, &tempData);

    char* ptr = g_assetStorage.Alloc<char>(strlen(filepath) + 1);
    strcpy_s(ptr, strlen(filepath) + 1, filepath);

    textureMap[ptr] = texture;
    return texture.id;
}

unsigned int Velox::AssetManager::GetTextureID(const char* filepath)
{
    for (auto pair : textureMap)
    {
        if (strcmp(pair.first, filepath) == 0)
            return pair.second.id;
    }

    printf("WARNING: Texture \'%s\' is not loaded\n", filepath);
    return 0;
}

unsigned int Velox::AssetManager::LoadShaderProgram(const char* vertFilepath, const char* fragFilepath, const char* name)
{
    for (auto pair : shaderProgramMap)
    {
        if (strcmp(pair.first, name) == 0)
            return pair.second.id;
    }

    Velox::Arena tempData(2048);

    // Haven't loaded texture before so send load for renderer to deal with.
    Velox::ShaderProgram shaderProgram {};
    shaderProgram = Velox::LoadShaderProgram(vertFilepath, fragFilepath);

    char* ptr = g_assetStorage.Alloc<char>(strlen(name) + 1);
    strcpy_s(ptr, strlen(name) + 1, name);

    shaderProgramMap[ptr] = shaderProgram;
    return shaderProgram.id;
}

unsigned int Velox::AssetManager::GetShaderProgramID(const char* name)
{
    for (auto pair : shaderProgramMap)
    {
        if (strcmp(pair.first, name) == 0)
            return pair.second.id;
    }

    printf("WARNING: Shader program \'%s\' is not loaded\n", name);
    return 0;
}

Velox::Font* Velox::AssetManager::LoadFont(const char* filepath)
{
    for (auto& pair : fontMap)
    {
        if (strcmp(pair.first, filepath) == 0)
            return &pair.second;
    }

    Velox::Arena tempData(2048);

    const size_t pathSize = 1024;
    char* absolutePath = tempData.Alloc<char>(pathSize);

    SDL_strlcpy(absolutePath, SDL_GetBasePath(), pathSize);
    SDL_strlcat(absolutePath, "assets\\fonts\\", pathSize);
    SDL_strlcat(absolutePath, filepath, pathSize);

    char* ptr = g_assetStorage.Alloc<char>(strlen(filepath) + 1);
    strcpy_s(ptr, strlen(filepath) + 1, filepath);

    fontMap[ptr] = {};
    Velox::Font& font = fontMap[ptr];

    font.name = ptr;

    msdfgen::FontHandle* ftFontHandle = msdfgen::loadFont(g_freetype, absolutePath);
    if (ftFontHandle == nullptr)
        throw std::runtime_error("Failed to load font");

    // Storage for glyph geometry and their coordinates in the atlas
    font.glyphs = std::vector<msdf_atlas::GlyphGeometry>();

    // FontGeometry is a helper class that loads a set of glyphs from a single font.
    // It can also be used to get additional font metrics, kerning information, etc.
    font.fontGeometry = msdf_atlas::FontGeometry(&font.glyphs);

    // Load a set of character glyphs:
    // The second argument can be ignored unless you mix different font sizes in one atlas.
    // In the last argument, you can specify a charset other than ASCII.
    // To load specific glyph indices, use loadGlyphs instead.
    font.fontGeometry.loadCharset(ftFontHandle, 1.0, msdf_atlas::Charset::ASCII);

    // Apply MSDF edge coloring. See edge-coloring.h for other coloring strategies.
    const double maxCornerAngle = 3.0;

    for (msdf_atlas::GlyphGeometry& glyph : font.glyphs)
        glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);

    msdf_atlas::TightAtlasPacker packer;

    // Set atlas parameters:
    // setDimensions or setDimensionsConstraint to find the best value
    packer.setDimensionsConstraint(msdf_atlas::DimensionsConstraint::SQUARE);

    // setScale for a fixed size or setMinimumScale to use the largest that fits
    packer.setMinimumScale(48.0);

    // setPixelRange or setUnitRange
    packer.setPixelRange(2.0);
    packer.setMiterLimit(1.0);

    // Compute atlas layout - pack glyphs
    packer.pack(font.glyphs.data(), font.glyphs.size());

    // Get final atlas dimensions
    int width = 0, height = 0;
    packer.getDimensions(width, height);

    // The ImmediateAtlasGenerator class facilitates the generation of the atlas bitmap.
    msdf_atlas::ImmediateAtlasGenerator<
        f32,                        // pixel type of buffer for individual glyphs depends on generator function
        3,                          // number of atlas color channels
        msdf_atlas::msdfGenerator,  // function to generate bitmaps for individual glyphs
        msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 3> // class that stores the atlas bitmap
    > generator(width, height);

    // GeneratorAttributes can be modified to change the generator's default settings.
    msdf_atlas::GeneratorAttributes attributes;
    generator.setAttributes(attributes);
    generator.setThreadCount(6);

    generator.generate(font.glyphs.data(), (int)font.glyphs.size());

    msdfgen::BitmapConstRef<msdf_atlas::byte, 3> bitmap = 
        (msdfgen::BitmapConstRef<msdf_atlas::byte, 3>)generator.atlasStorage();

// Not necessary to use a surface here, but we might want to do alterations using surface api.
#define USE_SURFACE 1

#if USE_SURFACE
    SDL_Surface* surface = 
        SDL_CreateSurfaceFrom(bitmap.width, bitmap.height, SDL_PIXELFORMAT_RGBX8888, (void*)bitmap.pixels, bitmap.width * 4);
    if (surface == nullptr)
    {
        printf("Error: Failed to generate font atlas surface\n");
        printf("Error: %s\n", SDL_GetError());
        throw std::runtime_error("Failed to generate surface from font atlas");
    }
#endif

    font.atlasResolution = ivec2(width, height);


    Velox::Texture fontTexture {};

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Generate texture
    glGenTextures(1, &fontTexture.id);
    glBindTexture(GL_TEXTURE_2D, fontTexture.id);
    unsigned int mipmapLevel = 0;

#if USE_SURFACE
    glTexImage2D(GL_TEXTURE_2D, mipmapLevel, GL_RGB8, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)surface->pixels);
#else
    glTexImage2D(GL_TEXTURE_2D, mipmapLevel, GL_RGB8, bitmap.width, bitmap.height, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)bitmap.pixels);
#endif

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Register texture
    textureMap[ptr] = fontTexture;

    // Register font
    font.textureId = fontTexture.id;

#if USE_SURFACE
    SDL_DestroySurface(surface);
#endif

    // Cleanup
    msdfgen::destroyFont(ftFontHandle);

    return &fontMap[ptr];
}

Velox::Font* Velox::AssetManager::GetFontRef(const char* filepath)
{
    for (auto& pair : fontMap)
    {
        if (strcmp(pair.first, filepath) == 0)
            return &pair.second;
    }

    printf("WARNING: Font \'%s\' is not loaded\n", filepath);
    return nullptr;
}

void Velox::AssetManager::DeInit()
{
    for (auto pair : textureMap)
        glDeleteTextures(1, &pair.second.id);

    for (auto pair : shaderProgramMap)
        glDeleteProgram(pair.second.id);
}

Velox::AssetManager* Velox::GetAssetManager()
{
    return &g_assetManager;
}

void Velox::InitAssets()
{
    g_freetype = msdfgen::initializeFreetype();
    if (g_freetype == nullptr)
        throw std::runtime_error("Failed to initialise freetype");
}

void Velox::DeInitAssets()
{
    g_assetManager.DeInit();
    msdfgen::deinitializeFreetype(g_freetype);
}

void Velox::GetAssetMemoryUsage(size_t* used, size_t* capacity)
{
    if (used)     *used     = g_assetStorage.offset;
    if (capacity) *capacity = g_assetStorage.size;
}

