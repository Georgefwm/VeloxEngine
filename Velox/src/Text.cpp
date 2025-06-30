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

    s_fontStack.push(requestedFont);
}

void Velox::PopFont()
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
void Velox::PushTextStyle(const Velox::TextDrawStyle& style)
{
    s_textStyleStack.push(style);
}

void Velox::PopTextStyle()
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

void Velox::GetStringContinueInfo(const char* text, Velox::TextContinueInfo* resultInfo, Velox::TextContinueInfo* startInfo)
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
    // Probably not going to work well if fonts are switched between DrawText calls.
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
        
        if (glyph == nullptr)
            glyph = fontGeometry.getGlyph('?'); // fallback char
        if (glyph == nullptr)
            printf("WARNING: Font is fucked m8\n");

        if (character == '\n')
        {
            x = 0;
            y -= fontScale * metrics.lineHeight + usingStyle->lineSpacing;
            continue;
        }

        // Update advance.

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

void Velox::GetStringBounds(const char* text, Velox::Rectangle* bounds, Velox::TextContinueInfo* textContinueInfo)
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
    // Probably not going to work well if fonts are switched between DrawText calls.
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
        
        if (glyph == nullptr)
            glyph = fontGeometry.getGlyph('?'); // fallback char
        if (glyph == nullptr)
            printf("WARNING: Font is fucked m8\n");

        if (character == '\n')
        {
            x = 0;
            y -= fontScale * metrics.lineHeight * usingStyle->lineSpacing;
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

        // Update advance.

        if (i < charCount - 1) // Last iteration.
        {
            double advance; 
            fontGeometry.getAdvance(advance, character, text[i + 1]);

            float kerningOffset = 0.0;
            x += fontScale * advance + kerningOffset;
        }
    }
}

void Velox::InitText()
{
    Velox::AssetManager* assetManager = Velox::GetAssetManager();

    g_defaultFont = assetManager->LoadFont("spicy_kebab.ttf");
}

