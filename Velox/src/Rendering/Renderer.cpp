#include "Rendering/Renderer.h"
#include <PCH.h>

#include "Arena.h"
#include "Asset.h"
#include "Velox.h"
#include "imgui.h"

#include <SDL3/SDL_surface.h>
#include <glad/gl.h> // Must be included before SDL
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3_image/SDL_image.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <fstream>

constexpr i32 WINDOW_WIDTH  = 1920;
constexpr i32 WINDOW_HEIGHT = 1080;

// Per frame max.
constexpr u32 MAX_QUADS    = 1024; // Everything we render is a quad.
constexpr u32 MAX_VERTICES = MAX_QUADS * 4;
constexpr u32 MAX_INDICES  = MAX_QUADS * 6;
constexpr u32 MAX_TEXTURES = 512;

constexpr char DEFAULT_SHADER_NAME[] = "default_shader";

SDL_Window* g_window;
SDL_GLContext g_glContext;

SDL_Window*    Velox::GetWindow()    { return  g_window;    }
SDL_GLContext* Velox::GetGLContext() { return &g_glContext; }

uint32_t g_currentFrame = 0;
bool g_frameBufferResized = false;

unsigned int g_vertexAttributeObject;
unsigned int g_vertexBufferObject;
unsigned int g_indexBufferObject;
unsigned int g_uniformBufferObject;

GLuint        g_vertexCount = 0;
Velox::Vertex g_vertices[MAX_VERTICES];

GLuint g_indexCount = 0;
GLuint g_indices[MAX_INDICES];

mat4 g_projectionMatrix;
mat4 g_viewMatrix;

GLuint g_drawCommandCount = 0;
std::vector<Velox::DrawCommand> g_drawCommands;

Velox::ShaderProgram g_defaultShaderProgram;
Velox::Texture g_errorTexture;

void CheckGLError()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
        printf("OpenGL error: 0x%x\n", err);
}

void Velox::ShaderProgram::Use() { glUseProgram(id); }
void Velox::Texture::Use() { glBindTexture(GL_TEXTURE_2D, id); }

ivec2 Velox::GetWindowSize()
{
    ivec2 size {};
    SDL_GetWindowSize(g_window, &size.x, &size.y);

    return size;
}

f32 Velox::GetDisplayScale()
{
    if (g_window == nullptr)
        return SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    return SDL_GetWindowDisplayScale(g_window);
}

void Velox::InitRenderer()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        throw std::runtime_error("Failed to init SDL");
    }

    const char* glsl_version = "#version 460";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    
    f32 displayScale = Velox::GetDisplayScale();

    SDL_WindowFlags windowFlags = 
        SDL_WINDOW_OPENGL |
        SDL_WINDOW_HIGH_PIXEL_DENSITY |
        SDL_WINDOW_HIDDEN;
    
    g_window = SDL_CreateWindow("GLProject", WINDOW_WIDTH, WINDOW_HEIGHT, windowFlags);
    if (g_window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        throw std::runtime_error("Failed to create SDL window");
    }

    g_glContext = SDL_GL_CreateContext(g_window);
    if (g_window == nullptr)
    {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        throw std::runtime_error("Failed to create GL context");
    }

    SDL_GL_MakeCurrent(g_window, g_glContext);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    SDL_SetWindowPosition(g_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(g_window);

    int version = gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);
    printf("GL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    const char* versionStr = (const char*)glGetString(GL_VERSION);
    printf("OpenGL Version: %s\n", versionStr);

    glClearColor(0, 0, 0, 1.0);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Render settings
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

    // glEnable(GL_CULL_FACE); 
    // glCullFace(GL_BACK);
    // glFrontFace(GL_CW); // Vertices are defind clockwise. This is standard in mordern model formats.

    glGenVertexArrays(1, &g_vertexAttributeObject);
    glGenBuffers(1, &g_vertexBufferObject);

    glGenBuffers(1, &g_indexBufferObject);

    glBindVertexArray(g_vertexAttributeObject);

    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, g_vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertices), g_vertices, GL_DYNAMIC_DRAW);
    glObjectLabel(GL_BUFFER, g_vertexBufferObject, -1, "Vertex Buffer");

    // Vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Velox::Vertex), (void*)offsetof(Velox::Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Velox::Vertex), (void*)offsetof(Velox::Vertex, color));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Velox::Vertex), (void*)offsetof(Velox::Vertex, uv));
    glEnableVertexAttribArray(2);

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_indices), g_indices, GL_DYNAMIC_DRAW);
    glObjectLabel(GL_BUFFER, g_indexBufferObject, -1, "Index Buffer");

    // Uniform buffer
    glGenBuffers(1, &g_uniformBufferObject);
    glBindBuffer(GL_UNIFORM_BUFFER, g_uniformBufferObject);

    // Allocate memory for UBO struct (2 mats + ivec2 + padding)
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Velox::UniformBufferObject), nullptr, GL_DYNAMIC_DRAW);

    // Bind UBO to binding point 0
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_uniformBufferObject);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glObjectLabel(GL_BUFFER, g_uniformBufferObject, -1, "Uniform Buffer");

    // Load default assets
    Velox::AssetManager* assetManager = Velox::GetAssetManager();

    g_defaultShaderProgram.id = assetManager->LoadShaderProgram(
        "shaders\\textured_quad.vert.glsl",
        "shaders\\textured_quad.frag.glsl",
        DEFAULT_SHADER_NAME);

    g_errorTexture.id = assetManager->LoadTexture("missing_texture.png");

    CheckGLError();
}

bool Velox::ForwardSDLEventToRenderer(SDL_Event* event)
{
    return false;
}

void Velox::StartFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    g_vertexCount = 0;
    g_indexCount = 0;
}

void Velox::DrawFrame()
{

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Velox::EndFrame()
{
    // GM: For now we just insert engine stuff here.
    // Mosly just drawing engine UI elements.
    Velox::DoFrameEndUpdates();

    // Generate ImGui stuff.
    ImGui::Render();

    // Copy our vertex data to GPU.
    Velox::DoCopyPass();

    // Draw out stuff.
    Velox::DoRenderPass();
}

void Velox::DoCopyPass()
{
    glBindBuffer(GL_ARRAY_BUFFER, g_vertexBufferObject);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_vertices), &g_vertices);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indexBufferObject);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(g_indices), &g_indices);

    // Uniform
    UniformBufferObject ubo {};
    ubo.projection = g_projectionMatrix;
    ubo.view = g_viewMatrix;
    SDL_GetWindowSize(g_window, &ubo.resolution.x, &ubo.resolution.y);

    glBindBuffer(GL_UNIFORM_BUFFER, g_uniformBufferObject);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UniformBufferObject), &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Velox::DoRenderPass()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_defaultShaderProgram.id);

    glBindVertexArray(g_vertexAttributeObject);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_uniformBufferObject);

    if (g_drawCommands.size() <= 0)
    {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(g_window);
        return;
    }

    Velox::DrawCommand& firstCommand = g_drawCommands[0];
    unsigned int currentShaderId  = firstCommand.shader.id;
    firstCommand.shader.Use();
    unsigned int currentTextureId = firstCommand.texture.id;
    firstCommand.texture.Use();

    unsigned int batchOffset = 0;
    unsigned int batchIndexCount = 0;
    for (int i = 0; i < g_drawCommands.size(); i++)
    {
        Velox::DrawCommand& command = g_drawCommands[i];

        int textureIdx = static_cast<int>(command.texture.id);
        int shaderIdx = static_cast<int>(command.shader.id);

        if (command.shader.id != currentShaderId || command.texture.id != currentTextureId)
        {
            if (currentShaderId <= 0) g_defaultShaderProgram.Use();
            if (currentTextureId <= 0) g_errorTexture.Use();

            // Submit batch of draws.
            glDrawElements(GL_TRIANGLES, batchIndexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)batchOffset);
            batchOffset = command.indexOffset * sizeof(unsigned int);
            batchIndexCount = 0;

            // Reset with new batch.
            command.shader.Use();
            currentShaderId = command.shader.id;
            command.texture.Use();
            currentTextureId = command.texture.id;
        }
        
        batchIndexCount += command.numIndices;
    }

    // Render last batch if any left.
    if (batchIndexCount > 0)
    {
        if (currentShaderId <= 0)  g_defaultShaderProgram.Use();
        if (currentTextureId <= 0) g_errorTexture.Use();

        glDrawElements(GL_TRIANGLES, batchIndexCount, GL_UNSIGNED_INT, (void*)(uintptr_t)batchOffset);
    }

    g_drawCommands.clear();

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(g_window);
}

char* LoadShaderFile(const char* filepath, size_t* byteSize, Velox::Arena* allocator)
{
    Velox::ShaderProgram shaderProgram {};

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

unsigned int CompileShader(char* shaderCode, int shaderStage, Velox::Arena* allocator)
{
    unsigned int shader = glCreateShader(shaderStage);
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

Velox::ShaderProgram Velox::LoadShaderProgram(const char* vertexFilepath, const char* fragmentFilepath)
{
    Velox::Arena tempData(100000);

    size_t vertCodeSize, fragCodeSize;
    char* vertCode = LoadShaderFile(vertexFilepath,   &vertCodeSize, &tempData);
    char* fragCode = LoadShaderFile(fragmentFilepath, &fragCodeSize, &tempData);

    unsigned int vertShader = CompileShader(vertCode, GL_VERTEX_SHADER,   &tempData);
    unsigned int fragShader = CompileShader(fragCode, GL_FRAGMENT_SHADER, &tempData);

    if (vertShader == 0 || fragShader == 0)
        throw std::runtime_error("Failed to compile shader");

    unsigned int id = glCreateProgram();
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

    // tempData.PrintUsage();

    return Velox::ShaderProgram { id };
}

Velox::Texture Velox::LoadTexture(const char* filepath, Velox::Arena* allocator)
{
    Velox::Texture texture {};

    const size_t pathSize = 1024;
    char* absolutePath = allocator->Alloc<char>(pathSize);

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

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);  

    unsigned int mipmapLevel = 0;

    glTexImage2D(GL_TEXTURE_2D, mipmapLevel, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    SDL_DestroySurface(surface);

    return texture;
}

void Velox::DeInitRenderer()
{
    glDeleteBuffers(1, &g_vertexBufferObject);
    glDeleteBuffers(1, &g_vertexAttributeObject);
    glDeleteBuffers(1, &g_indexBufferObject);
    glDeleteBuffers(1, &g_uniformBufferObject);

    glDeleteProgram(g_defaultShaderProgram.id);

    SDL_GL_DestroyContext(g_glContext);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
}

void Velox::Draw(std::vector<Velox::Vertex>& vertices, std::vector<GLuint>& indices,
        unsigned int textureId, unsigned int shaderId)
{
    if (vertices.size() + g_vertexCount > MAX_VERTICES)
    {
        printf("WARNING: Vertex frame limit reached\n");
        return;
    }

    if (indices.size() + g_indexCount > MAX_INDICES)
    {
        printf("WARNING: Index frame limit reached\n");
        return;
    }

    Velox::DrawCommand command {};

    command.indexOffset = g_indexCount;
    command.numIndices = indices.size();

    command.texture.id = textureId == 0 ? g_errorTexture.id         : textureId;
    command.shader.id  = shaderId  == 0 ? g_defaultShaderProgram.id : shaderId;

    for (int i = 0; i < indices.size(); i++)
        indices[i] += g_vertexCount;

    SDL_memcpy(&g_vertices[g_vertexCount], vertices.data(), sizeof(Velox::Vertex) * vertices.size());
    SDL_memcpy(&g_indices[g_indexCount], indices.data(), sizeof(GLuint) * indices.size());

    g_vertexCount += vertices.size();
    g_indexCount += indices.size();

    g_drawCommands.push_back(command);
}


