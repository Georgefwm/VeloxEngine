#include "Text.h"
#include <PCH.h>

#include "Asset.h"
#include <stack>
#include <msdf-atlas-gen/msdf-atlas-gen.h>

Velox::Font* g_defaultFont;
std::stack<Velox::Font*> g_fontStack;

void Velox::PushFont(const char* fontName)
{
    Velox::Font* requestedFont = Velox::GetAssetManager()->GetFontRef(fontName);
    if (requestedFont == nullptr)
    {
        printf("Font requested has not been loaded, please load it first\n");
        return;
    }

    // Don't allow same font to be pushed repeatedly. Simpler I think.
    if (g_fontStack.size() > 0)
    {
        if (SDL_strcmp(g_fontStack.top()->name, requestedFont->name) == 0)
            return;
    }

    g_fontStack.push(requestedFont);
}

void Velox::PopFont()
{
    if (g_fontStack.size() == 0)
    {
        return;
    }

    g_fontStack.pop();
}

Velox::Font* Velox::GetUsingFont()
{
    if (g_fontStack.size() == 0)
        return g_defaultFont;

    return g_fontStack.top();
}


void Velox::InitText()
{
    g_fontStack = {};

    Velox::AssetManager* assetManager = Velox::GetAssetManager();

    g_defaultFont = assetManager->LoadFont("spicy_kebab.ttf");
}

