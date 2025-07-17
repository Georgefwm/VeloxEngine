#include "Rendering/Renderer.h"
#include <PCH.h>

#include "Asset.h"
#include "Config.h"
#include "Event.h"
#include "Rendering/Pipeline.h"
#include "Text.h"
#include "Core.h"

#include <glad/gl.h> // Must be included before SDL

#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3_image/SDL_image.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

// Aparently windows uses this as 'default' scale.
constexpr float USER_DEFAULT_SCREEN_DPI = 96.0f;

constexpr u32 MAX_TEXTURES = 512;

constexpr char DEFAULT_SHADER_NAME[] = "default_shader";

constexpr u32  QUAD_VERTEX_INDICES[6]   = { 0, 1, 2, 2, 3, 0 };

// Can also be used for uvs.
//
// Quad verter order (clockwise from bottom left):
//   1--2
//   |  |
//   0--3
//
constexpr vec4 QUAD_VERTEX_POSITIONS[4] = {
    {  0.0f, 0.0f, 0.0f, 1.0f },
	{  0.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f, 1.0f, 0.0f, 1.0f },
	{  1.0f, 0.0f, 0.0f, 1.0f },
};

constexpr vec4 QUAD_UV_POSITIONS[4] = {
	{  0.0f, 1.0f, 0.0f, 1.0f },
    {  0.0f, 0.0f, 0.0f, 1.0f },
	{  1.0f, 0.0f, 0.0f, 1.0f },
	{  1.0f, 1.0f, 0.0f, 1.0f },
};

Velox::Config* s_config;
static bool s_adaptiveVsyncSupported = false;

static ivec2 s_windowSize;
static ivec2 s_frameBufferSize;

static int s_vsyncMode;

SDL_Window* g_window;
SDL_GLContext g_glContext;

SDL_Window* Velox::GetWindow()    { return  g_window;    }
void*       Velox::GetGLContext() { return &g_glContext; }

bool g_frameBufferResized = false;

u32 g_uniformBufferObject;

Velox::TexturedQuadPipeline g_texturedQuadPipeline;
Velox::LinePipeline         g_linePipeline;
Velox::FontPipeline         g_fontPipeline;

mat4 g_projection;
mat4 g_view;

u32 g_drawCommandCount = 0;
std::vector<Velox::DrawCommand> g_drawCommands;

Velox::ShaderProgram* g_defaultShaderProgram;
Velox::ShaderProgram* g_fontShaderProgram;
Velox::ShaderProgram* g_colorShaderProgram;

Velox::Texture* g_errorTexture;
Velox::Texture* g_whiteTexture;

u32 g_lineWidth = 2;

void checkGLError()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
        LOG_ERROR("OpenGL Error: err", err);
}

void Velox::ShaderProgram::use() { glUseProgram(id); }
void Velox::Texture::use() { glBindTexture(GL_TEXTURE_2D, id); }

ivec2 Velox::getWindowSize() { return s_windowSize; }
i32 Velox::getVsyncMode()    { return s_vsyncMode;  }

f32 Velox::getDisplayScale()
{
    if (g_window == nullptr)
        return SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    return SDL_GetWindowDisplayScale(g_window);
}

// Currently doesn't account for fullscreen stuff.
void Velox::setResolution(ivec2 newResolution)
{
    s_windowSize = newResolution;

    s_config->windowWidth = s_windowSize.x;
    s_config->windowHeight = s_windowSize.y;

    // Prevent changes pre-SDL_CreateWindow
    if (g_window == nullptr)
        return;

    SDL_SetWindowSize(g_window, s_windowSize.x, s_windowSize.y);
    SDL_SetWindowPosition(g_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    // Prevent being called pre-SDL_GL_CreateContext().
    if (g_glContext == nullptr)
        return;

    SDL_GetWindowSizeInPixels(g_window, &s_frameBufferSize.x, &s_frameBufferSize.y);

    // update OpenGL things.
    glViewport(0, 0, s_frameBufferSize.x, s_frameBufferSize.y);

    g_projection =  glm::ortho(0.0f, (float)s_frameBufferSize.x, (float)s_frameBufferSize.y, 0.0f, -1.0f, 1.0f);
}

void Velox::setVsyncMode(int newMode)
{
    if (!s_adaptiveVsyncSupported && newMode == -1)
    {
        LOG_WARN("Adaptive vsync is not supported on this machine, falling back to 'On'");
        s_vsyncMode = 1;
    }
    else
        s_vsyncMode = newMode;

    s_config->vsyncMode = s_vsyncMode; 

    // Prevent being called pre-SDL_GL_CreateContext().
    if (g_glContext != nullptr)
        SDL_GL_SetSwapInterval(s_vsyncMode);
}

bool Velox::isAdaptiveVsyncSupported() { return s_adaptiveVsyncSupported; }

void Velox::initRenderer()
{
    // Support checks
    s_adaptiveVsyncSupported = SDL_GL_ExtensionSupported("WGL_EXT_swap_control_tear"); // Windows
    // s_adaptiveVsyncSupported = SDL_GL_ExtensionSupported("GLX_EXT_swap_control"); // Linux

    // Apply config
    s_config = Velox::getConfig();
    Velox::setResolution(ivec2(s_config->windowWidth, s_config->windowHeight));
    Velox::setVsyncMode(s_config->vsyncMode);

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        LOG_CRITICAL("Failed to initialise SDL: {}", SDL_GetError());
        throw std::runtime_error("");
    }

    const char* glsl_version = "#version 460";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    
    f32 displayScale = Velox::getDisplayScale();

    SDL_WindowFlags windowFlags = 
        SDL_WINDOW_OPENGL |
        SDL_WINDOW_HIGH_PIXEL_DENSITY |
        SDL_WINDOW_HIDDEN;

    g_window = SDL_CreateWindow("GLProject", s_windowSize.x, s_windowSize.y, windowFlags);
    if (g_window == nullptr)
    {
        LOG_CRITICAL("Failed to create window: {}", SDL_GetError());
        throw std::runtime_error("");
    }

    g_glContext = SDL_GL_CreateContext(g_window);
    if (g_window == nullptr)
    {
        LOG_CRITICAL("Failed to create GL context: {}", SDL_GetError());
        throw std::runtime_error("");
    }

    SDL_GL_MakeCurrent(g_window, g_glContext);

    setVsyncMode(s_vsyncMode);

    SDL_SetWindowPosition(g_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(g_window);

    int version = gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);
    LOG_TRACE("Using GL Version {}", (const char*)glGetString(GL_VERSION));
    LOG_TRACE("GLAD loaded fns for version {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    SDL_GetWindowSizeInPixels(g_window, &s_frameBufferSize.x, &s_frameBufferSize.y);
    glViewport(0, 0, s_frameBufferSize.x, s_frameBufferSize.y);

    // Render settings
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

    // glEnable(GL_CULL_FACE); 
    // glCullFace(GL_BACK);
    // glFrontFace(GL_CW); // Vertices are defind clockwise. This is standard in mordern model formats.

    g_texturedQuadPipeline.init(1);
    g_linePipeline.init(2);
    g_fontPipeline.init(3);

    // Uniform buffer
    glGenBuffers(1, &g_uniformBufferObject);
    glBindBuffer(GL_UNIFORM_BUFFER, g_uniformBufferObject);

    // Allocate memory for UBO struct.
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Velox::UniformBufferObject), nullptr, GL_DYNAMIC_DRAW);

    // Bind UBO to binding point 0
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_uniformBufferObject);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glObjectLabel(GL_BUFFER, g_uniformBufferObject, -1, "Uniform Buffer");

    // Load default assets
    Velox::AssetManager* assetManager = Velox::getAssetManager();

    g_defaultShaderProgram = assetManager->loadShaderProgram(
        "shaders\\textured_quad.vert.glsl",
        "shaders\\textured_quad.frag.glsl",
        DEFAULT_SHADER_NAME);

    g_fontShaderProgram = assetManager->loadShaderProgram(
        "shaders\\sdf_quad.vert.glsl",
        "shaders\\sdf_quad.frag.glsl",
        "sdf_quad");

    g_colorShaderProgram = assetManager->loadShaderProgram(
        "shaders\\colored.vert.glsl",
        "shaders\\colored.frag.glsl",
        "color");

    g_errorTexture = assetManager->loadTexture("missing_texture.png");
    g_whiteTexture = assetManager->loadTexture("white.png");

    g_projection =  glm::ortho(0.0f, (float)s_windowSize.x, (float)s_windowSize.y, 0.0f, -1.0f, 1.0f);
    g_view = glm::mat4(1.0f);
    // g_view = glm::translate(g_view, glm::vec3(0.0f, 0.0f, -3.0f)); 
    
    Velox::SubscribeInfo subInfo {
        .name = "Renderer",
        .eventRangeStart = SDL_EVENT_DISPLAY_FIRST,
        .eventRangeEnd   = SDL_EVENT_WINDOW_LAST,  // Window events are just after display events.
        .callback = Velox::rendererEventCallback,
        .priority = 2,
    };

    Velox::getEventPublisher()->subscribe(subInfo);

    checkGLError();
}

bool Velox::rendererEventCallback(SDL_Event& event)
{
    return false;
}

void Velox::drawFrame()
{

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Velox::submitFrameData()
{
    // GM: For now we just insert engine stuff here.
    // Mosly just drawing engine UI elements.
    Velox::doFrameEndUpdates();

    // Generate ImGui stuff.
    ImGui::Render();

    // Copy our vertex data to GPU.
    Velox::doCopyPass();

    // Draw out stuff.
    Velox::doRenderPass();

    SDL_GL_SwapWindow(g_window);

    // Reset render frame state.
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    g_texturedQuadPipeline.clearFrameData();
    g_linePipeline.clearFrameData();
    g_fontPipeline.clearFrameData();
}

void Velox::doCopyPass()
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Copy Pass");

    g_texturedQuadPipeline.copyData();
    g_linePipeline.copyData();
    g_fontPipeline.copyData();

    // Uniform
    UniformBufferObject ubo {};
    ubo.projection = g_projection;
    ubo.view = g_view;
    SDL_GetWindowSize(g_window, &ubo.resolution.x, &ubo.resolution.y);

    glBindBuffer(GL_UNIFORM_BUFFER, g_uniformBufferObject);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UniformBufferObject), &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glPopDebugGroup();
}

void Velox::doRenderPass()
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Velox render Pass");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (g_drawCommands.size() <= 0)
    {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(g_window);
        return;
    }

    Velox::DrawCommand& firstCommand = g_drawCommands[0];

    Velox::Pipeline* currentPipeline = firstCommand.pipeline;
    currentPipeline->use(g_uniformBufferObject);

    u32 currentShaderID  = firstCommand.shader->id;
    firstCommand.shader->use();

    u32 currentTextureID = firstCommand.texture->id;
    firstCommand.texture->use();

    u32 batchOffset = 0;
    u32 batchIndexCount = 0;

    for (int i = 0; i < g_drawCommands.size(); i++)
    {
        Velox::DrawCommand& command = g_drawCommands[i];

        // int textureIdx = static_cast<int>(command.texture.id);
        // int shaderIdx = static_cast<int>(command.shader.id);

        if (command.pipeline->id != currentPipeline->id || 
            command.shader->id   != currentShaderID     ||
            command.texture->id  != currentTextureID)
        {
            if (currentShaderID  <= 0) g_defaultShaderProgram->use();
            if (currentTextureID <= 0) g_errorTexture->use();

            // Submit batch of draws.
            glDrawElements(currentPipeline->GLDrawType, batchIndexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)batchOffset);

            // update pipeline progress.
            // pipelineOffsets[currentPipeline->id] += batchIndexCount;

            // Reset with new batch.
            currentPipeline = command.pipeline;
            currentPipeline->use(g_uniformBufferObject);

            currentShaderID = command.shader->id;
            command.shader->use();

            currentTextureID = command.texture->id;
            command.texture->use();

            batchOffset = command.indexOffset * sizeof(u32);
            batchIndexCount = 0;

        }
        
        batchIndexCount += command.numIndices;
    }

    // Render last batch if any left.
    if (batchIndexCount > 0)
    {
        if (currentShaderID  <= 0)  g_defaultShaderProgram->use();
        if (currentTextureID <= 0) g_errorTexture->use();

        glDrawElements(currentPipeline->GLDrawType, batchIndexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)batchOffset);
    }

    g_drawCommands.clear();

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    glPopDebugGroup();

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "ImGui");
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glPopDebugGroup();
}

void Velox::deInitRenderer()
{
    g_texturedQuadPipeline.deInit();
    g_linePipeline.deInit();
    g_fontPipeline.deInit();

    glDeleteBuffers(1, &g_uniformBufferObject);

    glDeleteProgram(g_defaultShaderProgram->id);

    SDL_GL_DestroyContext(g_glContext);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
}

void Velox::drawQuad(const mat4& transform, const mat4& uvTransform, const vec4& color,
        Velox::Texture* texture, Velox::ShaderProgram* shader)
{
    const u32 startVertexOffset = g_texturedQuadPipeline.vertexCount;
    const u32 startIndexOffset = g_texturedQuadPipeline.indexCount;

    if (startIndexOffset + 6 > MAX_INDICES)
    {
        LOG_WARN("TexturedQuadPipeline is full, ignoring");
        return;
    }

    constexpr u32 quadVertexCount = 4;
    constexpr u32 quadIndexCount  = 6;

    Velox::DrawCommand command {};
    command.pipeline = &g_texturedQuadPipeline;
    command.texture  = texture != nullptr ? texture : g_errorTexture;
    command.shader   = shader  != nullptr ? shader  : g_defaultShaderProgram;
    command.indexOffset = startIndexOffset;
    command.numIndices  = quadIndexCount;  // Always 6 for a quad.

    for (u32 i = 0; i < quadVertexCount; i++)
    {
        Velox::TextureVertex vertex {};
        vertex.position = transform * QUAD_VERTEX_POSITIONS[i];
        vertex.color    = color;
        vertex.uv       = uvTransform * QUAD_UV_POSITIONS[i];

        g_texturedQuadPipeline.vertices[startVertexOffset + i] = vertex;
    }

    for (u32 i = 0; i < quadIndexCount; i++)
    {
        g_texturedQuadPipeline.indices[startIndexOffset + i] = 
            QUAD_VERTEX_INDICES[i] + startVertexOffset;
    }

    g_texturedQuadPipeline.vertexCount += quadVertexCount;
    g_texturedQuadPipeline.indexCount += quadIndexCount;

    g_drawCommands.push_back(command);
}

void Velox::drawQuad(const vec3& position, const vec2& size, const vec4& color,
        Velox::Texture* texture, Velox::ShaderProgram* shader)
{
    const mat4 transform = 
        glm::translate(glm::mat4(1.0f), position) *
        glm::scale(    glm::mat4(1.0f), vec3(size.x, size.y, 1.0f));

    const mat4 uvTransform = mat4(1.0f);

    Velox::drawQuad(transform, uvTransform, color, texture != nullptr ? texture : g_whiteTexture, shader);
}

void Velox::drawRotatedQuad(const vec3& position, const vec2& size, const vec4& color, 
        const f32& rotation, Velox::Texture* texture, Velox::ShaderProgram* shader)
{
    const vec3 pivotOffset = vec3(-0.5f * size.x, -0.5f * size.y, 0.0f);

    const mat4 transform = 
        glm::translate(mat4(1.0f), position) *
        glm::translate(mat4(1.0f), -pivotOffset) *
        glm::rotate(mat4(1.0f), glm::radians(rotation), vec3(0.0f, 0.0f, 1.0f)) *
        glm::translate(mat4(1.0f), pivotOffset) *
        glm::scale(mat4(1.0f), vec3(size.x, size.y, 1.0f));

    const mat4 uvTransform = mat4(1.0f);

    Velox::drawQuad(transform, uvTransform, color, texture, shader);
}

void Velox::drawQuadUV(const Velox::Rectangle& outRect, const Velox::Rectangle& inRect, 
        const vec4& color, Velox::Texture* texture, Velox::ShaderProgram* shader)
{

    const mat4 quadTransform = 
        glm::translate(glm::mat4(1.0f), vec3(outRect.x, outRect.y, 0.0f)) *
        glm::scale(    glm::mat4(1.0f), vec3(outRect.w, outRect.h, 1.0f));

    const mat4 uvTransform =
        glm::translate(glm::mat4(1.0f), vec3(inRect.x, inRect.y, 0.0f)) *
        glm::scale(    glm::mat4(1.0f), vec3(inRect.w, inRect.h, 1.0f));

    Velox::drawQuad(quadTransform, uvTransform, color, texture, shader);
}

void Velox::drawLine(const vec3& p0, const vec3& p1, const vec4& color)
{
    const u32 startVertexOffset = g_linePipeline.vertexCount;
    const u32 startIndexOffset  = g_linePipeline.indexCount;

    if (startIndexOffset + 6 > MAX_INDICES)
    {
        LOG_WARN("TexturedQuadPipeline is full, ignoring");
        return;
    }

    Velox::DrawCommand command {};
    command.pipeline = &g_linePipeline;
    command.texture  = g_whiteTexture;
    command.shader   = g_colorShaderProgram;
    command.indexOffset = startIndexOffset;
    command.numIndices  = 2;

    g_linePipeline.vertices[startVertexOffset + 0] = Velox::LineVertex {
        .position = p0,
        .color    = color,
    };

    g_linePipeline.vertices[startVertexOffset + 1] = Velox::LineVertex {
        .position = p1,
        .color    = color,
    };

    g_linePipeline.indices[startIndexOffset + 0] = startVertexOffset + 0;
    g_linePipeline.indices[startIndexOffset + 1] = startVertexOffset + 1;

    g_linePipeline.vertexCount += 2;
    g_linePipeline.indexCount += 2;

    g_drawCommands.push_back(command);
}

void Velox::drawRect(const Velox::Rectangle& rect, const vec4& color)
{
    glm::vec3 p0 = glm::vec3(rect.x,          rect.y,          0.0f);
    glm::vec3 p1 = glm::vec3(rect.x,          rect.y + rect.h, 0.0f);
    glm::vec3 p2 = glm::vec3(rect.x + rect.w, rect.y + rect.h, 0.0f);
    glm::vec3 p3 = glm::vec3(rect.x + rect.w, rect.y,          0.0f);

    Velox::drawLine(p0, p1, color);
    Velox::drawLine(p1, p2, color);
    Velox::drawLine(p2, p3, color);
    Velox::drawLine(p3, p0, color);
}

void Velox::drawRect(const vec3& position, const vec2& size, const vec4& color)
{
    drawRect(Velox::Rectangle { position.x, position.y, size.x, size.y }, color);
}

// GM: For reference of how fonts are rendered on screen see:
// https://freetype.org/freetype2/docs/tutorial/step2.html#section-1
Velox::TextContinueInfo Velox::drawText(const char* text, const vec3& position,
        Velox::TextContinueInfo* textContinueInfo)
{
    bool drawDebugLines = Velox::getEngineState()->drawTextLines;

    Velox::Font* usingFont = Velox::GetUsingFont();
    Velox::TextDrawStyle* usingStyle = Velox::GetUsingTextStyle();

    const msdf_atlas::FontGeometry& fontGeometry = usingFont->fontGeometry;

    msdfgen::FontMetrics metrics = fontGeometry.getMetrics();

    double x = 0.0;
    double fontScale = 1 / (metrics.ascenderY - metrics.descenderY);
    double y = fontScale * (metrics.ascenderY);

    Velox::Rectangle bounds {};
    bounds.x = 9999;
    bounds.y = 9999;

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

        u32 startVertexOffset = g_fontPipeline.vertexCount;
        u32 startIndexOffset  = g_fontPipeline.indexCount;

        if (startIndexOffset + 6 > MAX_INDICES)
            throw std::runtime_error("Font pipeline is full");

        constexpr u32 quadVertexCount = 4;
        constexpr u32 quadIndexCount  = 6;

        Velox::DrawCommand command {};
        command.pipeline = &g_fontPipeline;
        command.texture  = usingFont->texture;
        command.shader   = g_fontShaderProgram;
        command.indexOffset = startIndexOffset;
        command.numIndices  = quadIndexCount;  // Always 6 for a quad.

        glm::mat4 transform = 
            glm::translate(glm::mat4(1.0f), position) *
            glm::scale(glm::mat4(1.0f), vec3(vec2(usingStyle->textSize), 1.0f));

        Velox::FontVertex baseVertex = {
            .innerColor = usingStyle->color,
            .threshold  = usingStyle->fontWeightBias,
            .outBias    = 0.25f,
            .outerColor = usingStyle->outlineColor,
            .outlineWidthAbsolute = usingStyle->outlineWidth,
            .outlineWidthRelative = usingStyle->outlineWidth / 4,
            .outlineBlur = usingStyle->outlineBlur,
        };

        baseVertex.position = transform * vec4(quadMin.x, quadMin.y, 0.0f, 1.0f);
        baseVertex.uv       = { textureCoordMin.x, textureCoordMin.y };
        g_fontPipeline.vertices[startVertexOffset + 0] = baseVertex;

        baseVertex.position = transform * vec4(quadMin.x, quadMax.y, 0.0f, 1.0f);
        baseVertex.uv       = { textureCoordMin.x, textureCoordMax.y };
        g_fontPipeline.vertices[startVertexOffset + 1] = baseVertex;
        
        if (drawDebugLines)
        {
            if (bounds.x > baseVertex.position.x)
                bounds.x = baseVertex.position.x;

            if (bounds.y > baseVertex.position.y)
                bounds.y = baseVertex.position.y;
        }

        baseVertex.position = transform * vec4(quadMax.x, quadMax.y, 0.0f, 1.0f);
        baseVertex.uv       = { textureCoordMax.x, textureCoordMax.y };
        g_fontPipeline.vertices[startVertexOffset + 2] = baseVertex;

        baseVertex.position = transform * vec4(quadMax.x, quadMin.y, 0.0f, 1.0f);
        baseVertex.uv       = { textureCoordMax.x, textureCoordMin.y };
        g_fontPipeline.vertices[startVertexOffset + 3] = baseVertex;

        if (drawDebugLines)
        {
            if (bounds.w < baseVertex.position.x - bounds.x)
                bounds.w = baseVertex.position.x - bounds.x;

            if (bounds.h < baseVertex.position.y - bounds.y)
                bounds.h = baseVertex.position.y - bounds.y;
        }

        for (u32 i = 0; i < quadIndexCount; i++)
        {
            g_fontPipeline.indices[startIndexOffset + i] = 
                QUAD_VERTEX_INDICES[i] + startVertexOffset;
        }

        g_fontPipeline.vertexCount += quadVertexCount;
        g_fontPipeline.indexCount  += quadIndexCount;

        g_drawCommands.push_back(command);

        // update advance.

        if (i < charCount - 1) // Last iteration.
        {
            double advance; 
            fontGeometry.getAdvance(advance, character, text[i + 1]);

            float kerningOffset = 0.0;
            x += fontScale * advance + kerningOffset;
        }
    }

    if (drawDebugLines)
    {
        Velox::drawRect(bounds, COLOR_GREEN);

        // Baseline
        Velox::drawLine(vec3(bounds.x, position.y + (metrics.ascenderY * usingStyle->textSize), 0.0f),
                vec3(bounds.x + bounds.w, position.y + (metrics.ascenderY * usingStyle->textSize), 0.0f),
                COLOR_RED);

        Velox::drawLine(vec3(position), vec3(position) + vec3(0.0f, 40.0f, 0.0f), COLOR_YELLOW);
        Velox::drawLine(vec3(position), vec3(position) + vec3(40.0f, 0.0f, 0.0f), COLOR_YELLOW);
    }

    return Velox::TextContinueInfo { 
        .lastChar = text[charCount - 1],
        .advanceX = x,
        .advanceY = y,
    };
}


Velox::TextContinueInfo Velox::drawColoredText(const char* text, const vec3& position,
        const vec4& color, Velox::TextContinueInfo* textContinueInfo)
{
    Velox::TextDrawStyle* style = Velox::GetUsingTextStyle();
    style->color = color;

    Velox::pushTextStyle(*style);
    Velox::TextContinueInfo continueInfo = Velox::drawText(text, position, textContinueInfo);
    Velox::popTextStyle();

    return continueInfo;
}
