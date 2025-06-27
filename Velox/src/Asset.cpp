#include "Asset.h"
#include <PCH.h>

#include "Arena.h"
#include "Rendering/Renderer.h"

#include <SDL3_image/SDL_image.h>
#include <msdf-atlas-gen/types.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_surface.h>
#include <glad/gl.h>
#include <msdf-atlas-gen/msdf-atlas-gen.h>

#include <fstream>

static Velox::Arena g_assetStorage(1024);
static Velox::AssetManager g_assetManager {};
static msdfgen::FreetypeHandle* g_freetype;

u32 Velox::AssetManager::LoadTexture(const char* filepath)
{
    for (auto pair : textureMap)
    {
        if (strcmp(pair.first, filepath) == 0)
            return pair.second.id;
    }

    Velox::Arena tempData(2048);
    // Haven't loaded texture before so send load for renderer to deal with.

    const size_t pathSize = 1024;
    char* absolutePath = tempData.Alloc<char>(pathSize);

    SDL_strlcpy(absolutePath, SDL_GetBasePath(), pathSize);
    SDL_strlcat(absolutePath, "assets\\textures\\", pathSize);
    SDL_strlcat(absolutePath, filepath, pathSize);

    SDL_Surface* surface = IMG_Load(absolutePath);
    if (surface == nullptr)
    {
        printf("Error: Failed to load image from path: %s\n", absolutePath);
        printf("Error: %s\n", SDL_GetError());
        throw std::runtime_error("Failed to load image from disk");
    }

    if (surface->format != SDL_PIXELFORMAT_ABGR8888)
    {
        SDL_Surface* convertedSurface;
        convertedSurface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ABGR8888);

        if (convertedSurface == nullptr)
        {
            printf("Error: Loaded image has wrong pixel format and couldn't convert. \
                    Expected \"SDL_PIXELFORMAT_ABGR888*\", \
                found: \"%s\"\n", SDL_GetPixelFormatName(surface->format));

            throw std::runtime_error("Loaded image has wrong pixel format.");
        }

        SDL_DestroySurface(surface);
        surface = convertedSurface;
    }

    // SDL loads images upside down (think this is standard for non-opengl rendering APIs).
    SDL_FlipSurface(surface, SDL_FLIP_VERTICAL);

    u32 id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);  
    glObjectLabel(GL_TEXTURE, id, -1, filepath);

    u32 mipmapLevel = 0;

    glTexImage2D(GL_TEXTURE_2D, mipmapLevel, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    SDL_DestroySurface(surface);

    char* ptr = g_assetStorage.Alloc<char>(strlen(filepath) + 1);
    strcpy_s(ptr, strlen(filepath) + 1, filepath);

    textureMap[ptr] = { id };

    return id;
}

u32 Velox::AssetManager::GetTextureID(const char* filepath)
{
    for (auto pair : textureMap)
    {
        if (strcmp(pair.first, filepath) == 0)
            return pair.second.id;
    }

    printf("WARNING: Texture \'%s\' is not loaded\n", filepath);
    return 0;
}

char* LoadShaderFile(const char* filepath, size_t* byteSize, Velox::Arena* allocator)
{
    const size_t pathSize = 1024;
    char* absolutePath = allocator->Alloc<char>(pathSize);

    SDL_strlcpy(absolutePath, SDL_GetBasePath(), pathSize);
    SDL_strlcat(absolutePath, filepath, pathSize);

    std::ifstream file(absolutePath, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        printf("No file found at \'%s\'\n", absolutePath);
        throw std::runtime_error("failed to open file");
    }

    size_t fileSize = (size_t)file.tellg();
    char* shaderCode = allocator->Alloc<char>(fileSize + 1);

    file.seekg(0);
    file.read(shaderCode, fileSize);

    shaderCode[fileSize] = '\0';

    file.close();

    *byteSize = fileSize;
    return shaderCode;
}

u32 CompileShader(char* shaderCode, int shaderStage, Velox::Arena* allocator)
{
    u32 shader = glCreateShader(shaderStage);
    glShaderSource(shader, 1, &shaderCode, NULL);
    glCompileShader(shader);

    char* logData = allocator->Alloc<char>(1024);

    int result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    if(!result)
    {
        glGetShaderInfoLog(shader, 512, NULL, logData);
        printf("Shader failed to compile: %s\n", logData);
        glDeleteShader(shader); // don't keep a bad shader
        return 0;
    };

    return shader;
}

u32 Velox::AssetManager::LoadShaderProgram(const char* vertFilepath, const char* fragFilepath, const char* name)
{
    for (auto pair : shaderProgramMap)
    {
        if (strcmp(pair.first, name) == 0)
            return pair.second.id;
    }

    // Might need more if we have a big shader to load.
    Velox::Arena tempData(100000);

    size_t vertCodeSize, fragCodeSize;
    char* vertCode = LoadShaderFile(vertFilepath, &vertCodeSize, &tempData);
    char* fragCode = LoadShaderFile(fragFilepath, &fragCodeSize, &tempData);

    u32 vertShader = CompileShader(vertCode, GL_VERTEX_SHADER,   &tempData);
    u32 fragShader = CompileShader(fragCode, GL_FRAGMENT_SHADER, &tempData);

    if (vertShader == 0 || fragShader == 0)
        throw std::runtime_error("Failed to compile shader");

    u32 id = glCreateProgram();
    glAttachShader(id, vertShader);
    glAttachShader(id, fragShader);
    glLinkProgram(id);

    // print linking errors if any
    char* logData = tempData.Alloc<char>(2048);

    int result;
    glGetProgramiv(id, GL_LINK_STATUS, &result);
    if(!result)
    {
        glGetProgramInfoLog(id, 512, NULL, logData);
        printf("Shader failed to create shader module: %s\n", logData);
    }
      
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    
    glObjectLabel(GL_PROGRAM, id, -1, name);

    // Register shader
    char* ptr = g_assetStorage.Alloc<char>(strlen(name) + 1);
    strcpy_s(ptr, strlen(name) + 1, name);

    shaderProgramMap[ptr] = { id };

    return id;
}

u32 Velox::AssetManager::GetShaderProgramID(const char* name)
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
    packer.setMinimumScale(40.0);

    // setPixelRange or setUnitRange
    packer.setPixelRange(4.0);
    packer.setMiterLimit(1.0);
    packer.setOuterPixelPadding(2.0);

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
    glObjectLabel(GL_TEXTURE, fontTexture.id, -1, filepath);
    u32 mipmapLevel = 0;

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

