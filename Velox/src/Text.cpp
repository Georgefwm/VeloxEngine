#include "Text.h"
#include <PCH.h>

#include "Asset.h"
#include <SDL3/SDL_stdinc.h>
#include <glm/ext/matrix_transform.hpp>
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
    double fontScale = 1 / (metrics.ascenderY - metrics.descenderY);
    double y = fontScale * (metrics.ascenderY);

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

void Velox::getStringBounds(const char* text, const vec3& position, Velox::Rectangle* bounds,
        Velox::TextContinueInfo* textContinueInfo)
{
    if (bounds == nullptr)
        return;

    Velox::Font* usingFont = Velox::GetUsingFont();
    Velox::TextDrawStyle* usingStyle = Velox::GetUsingTextStyle();

    const msdf_atlas::FontGeometry& fontGeometry = usingFont->fontGeometry;

    msdfgen::FontMetrics metrics = fontGeometry.getMetrics();

    double x = 0.0;
    double fontScale = 1 / (metrics.ascenderY - metrics.descenderY);
    double y = fontScale * (metrics.ascenderY);

    bounds->x = 9999;
    bounds->y = 9999;

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
            y += usingStyle->textSize * metrics.lineHeight * usingStyle->lineSpacing;
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

        double atlasLeft, atlasBot, atlasRight, atlasTop;
        glyph->getQuadAtlasBounds(atlasLeft, atlasBot, atlasRight, atlasTop);

        vec2 textureCoordMin((f32)atlasLeft,  (f32)atlasBot);
        vec2 textureCoordMax((f32)atlasRight, (f32)atlasTop);

        double planeLeft, planeBot, planeRight, planeTop;
        glyph->getQuadPlaneBounds(planeLeft, planeBot, planeRight, planeTop);

        vec2 quadMin((f32)planeLeft,  (f32)planeTop);
        vec2 quadMax((f32)planeRight, (f32)planeBot);
        
        float yOffset = planeTop + planeBot;
        quadMin.y -= yOffset;
        quadMax.y -= yOffset;

        quadMax *= fontScale;
        
        vec2 currentAdvance((f32)x, (f32)y); 
        quadMin += currentAdvance;
        quadMax += currentAdvance;

        vec2 texelSize(1.0 / usingFont->atlasResolution.x, 1.0 / usingFont->atlasResolution.y);
        textureCoordMin *= texelSize;
        textureCoordMax *= texelSize;

        // Draw. 

        constexpr u32 quadVertexCount = 4;
        constexpr u32 quadIndexCount  = 6;

        glm::mat4 transform = 
            glm::translate(glm::mat4(1.0f), position) *
            glm::scale(glm::mat4(1.0f), vec3(vec2(usingStyle->textSize), 1.0f));

        Velox::FontVertex baseVertex;

        baseVertex.position = transform * vec4(quadMin.x, quadMax.y, 0.0f, 1.0f);
        
        if (bounds->x > baseVertex.position.x)
            bounds->x = baseVertex.position.x;

        if (bounds->y > baseVertex.position.y)
            bounds->y = baseVertex.position.y;

        baseVertex.position = transform * vec4(quadMax.x, quadMin.y, 0.0f, 1.0f);

        if (bounds->w < baseVertex.position.x - bounds->x)
            bounds->w = baseVertex.position.x - bounds->x;

        if (bounds->h < baseVertex.position.y - bounds->y)
            bounds->h = baseVertex.position.y - bounds->y;

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

