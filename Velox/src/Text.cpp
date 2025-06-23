#include "Text.h"
#include <PCH.h>

#include "Asset.h"
#include <stack>
#include <msdf-atlas-gen/msdf-atlas-gen.h>

static unsigned int g_fontShaderId;
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

Velox::Font* GetUsingFont()
{
    if (g_fontStack.size() == 0)
        return g_defaultFont;

    return g_fontStack.top();
}

// GM: For reference of how fonts are organised on screen see:
// https://freetype.org/freetype2/docs/tutorial/step2.html#section-1
void Velox::DrawText(const char* text, Velox::TextDrawInfo textDrawInfo)
{
    Velox::Font* usingFont = GetUsingFont();

    const msdf_atlas::FontGeometry& fontGeometry = usingFont->fontGeometry;

    msdfgen::FontMetrics metrics = fontGeometry.getMetrics();

    double x = 0.0;
    double fontScale = 1 / (metrics.ascenderY - metrics.descenderY) * textDrawInfo.textSize;
    double y = 0.0;
    
    size_t charCount = SDL_strlen(text);
    for (size_t i = 0; i < charCount; i++)
    {
        char character = text[i];

        const msdf_atlas::GlyphGeometry* glyph = fontGeometry.getGlyph(character);
        
        if (glyph == nullptr)
            glyph = fontGeometry.getGlyph('?'); // fallback char
        if (glyph == nullptr)
            printf("WARNING: Font is fucked m8\n");

        double atlasLeft, atlasBot, atlasRight, atlasTop;
        glyph->getQuadAtlasBounds(atlasLeft, atlasBot, atlasRight, atlasTop);

        vec2 textureCoordMin((f32)atlasLeft,  (f32)atlasBot);
        vec2 textureCoordMax((f32)atlasRight, (f32)atlasTop);

        double planeLeft, planeBot, planeRight, planeTop;
        glyph->getQuadPlaneBounds(planeLeft, planeBot, planeRight, planeTop);

        vec2 quadMin((f32)planeLeft,  (f32)planeBot);
        vec2 quadMax((f32)planeRight, (f32)planeTop);
        
        quadMin *= fontScale;
        quadMax *= fontScale;
        
        vec2 currentAdvance((f32)x, (f32)y); 
        quadMin += currentAdvance;
        quadMax += currentAdvance;

        vec2 texelSize(1.0 / usingFont->atlasResolution.x, 1.0 / usingFont->atlasResolution.y);
        textureCoordMin *= texelSize;
        textureCoordMax *= texelSize;

        // draw 

        quadMin += vec2(textDrawInfo.position);
        quadMax += vec2(textDrawInfo.position);

        std::vector<Velox::Vertex> vertices(4);
        
        vertices[0].position = vec3(quadMin.x, quadMin.y, 0.0);
        vertices[1].position = vec3(quadMax.x, quadMin.y, 0.0);
        vertices[2].position = vec3(quadMin.x, quadMax.y, 0.0);
        vertices[3].position = vec3(quadMax.x, quadMax.y, 0.0);

        vertices[0].color = textDrawInfo.color;
        vertices[1].color = textDrawInfo.color;
        vertices[2].color = textDrawInfo.color;
        vertices[3].color = textDrawInfo.color;

        vertices[0].uv = vec2(textureCoordMin.x, textureCoordMin.y);
        vertices[1].uv = vec2(textureCoordMax.x, textureCoordMin.y);
        vertices[2].uv = vec2(textureCoordMin.x, textureCoordMax.y);
        vertices[3].uv = vec2(textureCoordMax.x, textureCoordMax.y);

        std::vector<unsigned int> indices = { 0, 1, 2, 2, 1, 3 };

        Velox::Draw(vertices, indices, usingFont->textureId, g_fontShaderId);

        double advance = glyph->getAdvance(); 

        if (i < charCount - 1)
            fontGeometry.getAdvance(advance, character, text[i + 1]);

        float kerningOffset = 0.0;
        x += fontScale * advance + kerningOffset;
    }
}

void Velox::InitText()
{
    g_fontStack = {};

    Velox::AssetManager* assetManager = Velox::GetAssetManager();

    g_defaultFont = assetManager->LoadFont("spicy_kebab.ttf");
    g_fontShaderId = assetManager->LoadShaderProgram(
        "shaders\\sdf_quad.vert.glsl",
        "shaders\\sdf_quad.frag.glsl",
        "sdf_quad");

}

