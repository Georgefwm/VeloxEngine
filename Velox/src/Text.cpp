#include "Text.h"
#include <PCH.h>

#include "Asset.h"
#include <SDL3/SDL_stdinc.h>
#include <stack>
#include <msdf-atlas-gen/msdf-atlas-gen.h>

static Velox::Font* g_defaultFont;
static std::stack<Velox::Font*> s_fontStack {};

static Velox::TextDrawStyle s_defaultTextStyle {};
static std::stack<Velox::TextDrawStyle> s_textStyleStack {};

void Velox::pushFont(const char* fontName)
{
    Velox::Font* requestedFont = Velox::getAssetManager()->getFontRef(fontName);
    if (requestedFont == nullptr)
        return;

    s_fontStack.push(requestedFont);
}

void Velox::popFont()
{
    if (s_fontStack.size() == 0)
        return;

    s_fontStack.pop();
}

Velox::Font* Velox::GetUsingFont()
{
    if (s_fontStack.size() == 0)
        return g_defaultFont;

    return s_fontStack.top();
}

// GM: Would be nice to push/pop indiviudal styles, quick n dirty for now :)
void Velox::pushTextStyle(const Velox::TextDrawStyle& style)
{
    s_textStyleStack.push(style);
}

void Velox::popTextStyle()
{
    if (s_textStyleStack.size() == 0)
        return;

    s_textStyleStack.pop();
}
Velox::TextDrawStyle* Velox::GetUsingTextStyle()
{
    if (s_textStyleStack.size() == 0)
        return &s_defaultTextStyle;

    return &s_textStyleStack.top();
}

void Velox::getStringContinueInfo(const char* text, Velox::TextContinueInfo* resultInfo, Velox::TextContinueInfo* startInfo)
{
    Velox::Font* usingFont = Velox::GetUsingFont();
    Velox::TextDrawStyle* usingStyle = Velox::GetUsingTextStyle();

    const msdf_atlas::FontGeometry& fontGeometry = usingFont->fontGeometry;

    msdfgen::FontMetrics metrics = fontGeometry.getMetrics();

    double x = 0.0;
    double fontScale = 1 / (metrics.ascenderY - metrics.descenderY) * usingStyle->textSize;
    double y = 0.0;

    size_t charCount = SDL_strlen(text);

    // Info conintue info is given then resume advance positions.
    // Probably not going to work well if fonts are switched between drawText calls.
    if (startInfo != nullptr && charCount > 0)
    {
        x = startInfo->advanceX;
        y = startInfo->advanceY;

        double advance;
        fontGeometry.getAdvance(advance, startInfo->lastChar, text[0]);

        x += fontScale * advance;
    }
    
    for (size_t i = 0; i < charCount; i++)
    {
        char character = text[i];

        const msdf_atlas::GlyphGeometry* glyph = fontGeometry.getGlyph(character);
        
        if (character == '\n')
        {
            x = 0;
            y -= fontScale * metrics.lineHeight + usingStyle->lineSpacing;
            continue;
        }

        if (glyph == nullptr)
        {
            LOG_WARN("Couldn't find glyph for '{}', falling back to '?'", character);
            glyph = fontGeometry.getGlyph('?'); // fallback char
        }
        
        if (glyph == nullptr)
        {
            LOG_ERROR("Couldn't find fallback glyph");
            continue;
        }

        // update advance.

        if (i < charCount - 1) // Last iteration.
        {
            double advance; 
            fontGeometry.getAdvance(advance, character, text[i + 1]);

            x += fontScale * advance;
        }
    }

    resultInfo->lastChar = text[charCount - 1];
    resultInfo->advanceX = x;
    resultInfo->advanceY = y;
}

void Velox::getStringBounds(const char* text, Velox::Rectangle* bounds, Velox::TextContinueInfo* textContinueInfo)
{
    if (bounds == nullptr)
        return;

    *bounds = {};

    Velox::Font* usingFont = Velox::GetUsingFont();
    Velox::TextDrawStyle* usingStyle = Velox::GetUsingTextStyle();

    const msdf_atlas::FontGeometry& fontGeometry = usingFont->fontGeometry;

    msdfgen::FontMetrics metrics = fontGeometry.getMetrics();

    double x = 0.0;
    double fontScale = 1 / (metrics.ascenderY - metrics.descenderY) * usingStyle->textSize;
    double y = 0.0;

    size_t charCount = SDL_strlen(text);

    // Info conintue info is given then resume advance positions.
    // Probably not going to work well if fonts are switched between drawText calls.
    if (textContinueInfo != nullptr && charCount > 0)
    {
        x = textContinueInfo->advanceX;
        y = textContinueInfo->advanceY;

        double advance;
        fontGeometry.getAdvance(advance, textContinueInfo->lastChar, text[0]);

        x += fontScale * advance;
    }

    for (size_t i = 0; i < charCount; i++)
    {
        char character = text[i];

        const msdf_atlas::GlyphGeometry* glyph = fontGeometry.getGlyph(character);
        
        if (character == '\n')
        {
            x = 0;
            y -= fontScale * metrics.lineHeight + usingStyle->lineSpacing;
            continue;
        }

        if (glyph == nullptr)
        {
            LOG_WARN("Couldn't find glyph for '{}', falling back to '?'", character);
            glyph = fontGeometry.getGlyph('?'); // fallback char
        }
        
        if (glyph == nullptr)
        {
            LOG_ERROR("Couldn't find fallback glyph");
            continue;
        }

        // Only need to calculate the quad bounds for this.

        double planeLeft, planeBot, planeRight, planeTop;
        glyph->getQuadPlaneBounds(planeLeft, planeBot, planeRight, planeTop);

        vec2 quadMin((f32)planeLeft,  (f32)planeBot);
        vec2 quadMax((f32)planeRight, (f32)planeTop);
        
        quadMin *= fontScale;
        quadMax *= fontScale;
        
        vec2 currentAdvance((f32)x, (f32)y); 
        quadMin += currentAdvance;
        quadMax += currentAdvance;

        if (quadMin.x < bounds->x) bounds->x = quadMin.x;
        if (quadMin.y < bounds->y) bounds->y = quadMin.y;
        if (quadMax.x > bounds->w) bounds->w = quadMax.x;
        if (quadMax.y > bounds->h) bounds->h = quadMax.y;

        // update advance.

        if (i < charCount - 1) // Last iteration.
        {
            double advance; 
            fontGeometry.getAdvance(advance, character, text[i + 1]);

            float kerningOffset = 0.0;
            x += fontScale * advance + kerningOffset;
        }
    }
}

void Velox::initText()
{
    Velox::AssetManager* assetManager = Velox::getAssetManager();

    g_defaultFont = assetManager->loadFont("spicy_kebab.ttf");
}

