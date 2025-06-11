#include "Asset.h"
#include "Renderer.h"

static Velox::Arena g_assetStorage(10000);
static Velox::AssetManager g_assetManager {};

char defaultTexture[] = "missing_texture.png";

int Velox::AssetManager::LoadTexture(const char* filepath)
{
    for (auto pair : textureMap)
    {
        if (strcmp(pair.first, filepath) == 0)
            return pair.second;
    }

    // Haven't loaded texture before so send load for renderer to deal with.
    int newIndex = Velox::LoadTextureInternal(filepath);
    if (newIndex == 0)
    {
        printf("Error: Couldn't load texture.\n");
        return newIndex;
    }

    char* ptr = g_assetStorage.Alloc<char>(strlen(filepath) + 1);
    strcpy_s(ptr, strlen(filepath) + 1, filepath);

    textureMap[ptr] = newIndex;

    return newIndex;
}

int Velox::AssetManager::GetTextureIndex(const char* filepath)
{
    for (auto pair : textureMap)
    {
        if (strcmp(pair.first, filepath) == 0)
            return pair.second;
    }

    printf("WARNING: Texture \"%s\" is not loaded\n", filepath);
    return 0;
}

void Velox::AssetManager::DeInit()
{

}

Velox::AssetManager* Velox::GetAssetManager()
{
    return &g_assetManager;
}

void Velox::InitAssets()
{
}



void Velox::GetAssetMemoryUsage(size_t* used, size_t* capacity)
{
    *used     = 1;
    *capacity = 2;
}

