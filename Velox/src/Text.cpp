#include "Text.h"
#include <PCH.h>

#include "Asset.h"
#include <stack>
#include <msdf-atlas-gen/msdf-atlas-gen.h>

static Velox::Font* g_defaultFont;
static std::stack<Velox::Font*> s_fontStack {};

static Velox::TextDrawStyle s_defaultTextStyle {};
static std::stack<Velox::TextDrawStyle> s_textStyleStack {};

void Velox::PushFont(const char* fontName)
{
    Velox::Font* requestedFont = Velox::GetAssetManager()->GetFontRef(fontName);
    if (requestedFont == nullptr)
    {
        printf("Font requested has not been loaded, please load it first\n");
        return;
    }

    // Don't allow same font to be pushed repeatedly. Simpler I think.
    if (s_fontStack.size() > 0)
    {
        if (SDL_strcmp(s_fontStack.top()->name, requestedFont->name) == 0)
            return;
    }

    s_fontStack.push(requestedFont);
}

void Velox::PopFont()
{
    if (s_fontStack.size() == 0)
    {
        return;
    }

    s_fontStack.pop();
}

Velox::Font* Velox::GetUsingFont()
{
    if (s_fontStack.size() == 0)
        return g_defaultFont;

    return s_fontStack.top();
}

// GM: Would be nice to push/pop indiviudal styles, quick n dirty for now :)
void Velox::PushTextStyle(const Velox::TextDrawStyle& style)
{
    // Don't allow same style to be pushed twice.
    if (s_textStyleStack.size() > 0)
    {
        if (style == s_textStyleStack.top())
            return;
    }

    s_textStyleStack.push(style);
}

void Velox::PopTextStyle()
{
    if (s_textStyleStack.size() == 0)
    {
        return;
    }

    s_textStyleStack.pop();
}
Velox::TextDrawStyle* Velox::GetUsingTextStyle()
{
    if (s_textStyleStack.size() == 0)
        return &s_defaultTextStyle;

    return &s_textStyleStack.top();
}


void Velox::InitText()
{
    Velox::AssetManager* assetManager = Velox::GetAssetManager();

    g_defaultFont = assetManager->LoadFont("spicy_kebab.ttf");
}

