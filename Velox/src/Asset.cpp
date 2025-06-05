#include "Asset.h"

static Velox::AssetManager g_assetManager {};

void Velox::AssetManager::DeInit()
{

}

void Velox::InitAssets()
{
}

void Velox::LoadTexture(const char& filepath)
{

}

void Velox::GetAssetMemoryUsage(size_t* used, size_t* capacity)
{
    *used     = 1;
    *capacity = 2;
}

