#include "Renderer.h"

#include "SDL3/SDL_filesystem.h"
#include "SDL3/SDL_pixels.h"
#include "UI.h"
#include "Velox.h"

#include <cstddef>
#include <glm/glm.hpp>
#include <SDL3/SDL_gpu.h>
#include <SDL3_image/SDL_image.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include <cstdio>

constexpr size_t VERTEX_BUFFER_SIZE = 512;
constexpr size_t INDEX_BUFFER_SIZE  = 512;

constexpr SDL_FColor CLEAR_COLOR = { 0.5, 0.5, 0.5, 1.0 };

SDL_Window*    g_window;
SDL_GPUDevice* g_device;

Uint32 vertexCount = 0;
Uint32 indexCount = 0;

Velox::Vertex g_vertices[VERTEX_BUFFER_SIZE];
Uint32        g_indices[INDEX_BUFFER_SIZE];

SDL_GPUTransferBuffer* g_vertexTransferBuffer;
SDL_GPUTransferBuffer* g_indexTransferBuffer;

SDL_GPUBuffer* g_vertexBuffer;
SDL_GPUBuffer* g_indexBuffer;

SDL_GPUTexture* defaultTexture;
SDL_GPUSampler* g_sampler;

SDL_GPUGraphicsPipeline* g_graphicsPipeline;

SDL_Window*    Velox::GetWindow() { return g_window; }
SDL_GPUDevice* Velox::GetDevice() { return g_device; }

ivec2 Velox::GetWindowSize()
{
    ivec2 size {};
    SDL_GetWindowSize(g_window, &size.x, &size.y);

    return size;
}

float Velox::GetDisplayScale()
{
    return SDL_GetWindowDisplayScale(g_window);
}

bool Velox::InitRenderer()
{
    SDL_WindowFlags windowFlags;
    windowFlags &= SDL_WINDOW_VULKAN;
    windowFlags &= SDL_WINDOW_HIGH_PIXEL_DENSITY;
    
    g_window = SDL_CreateWindow("Velox App", 1920, 1080, windowFlags);
    if (g_window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return false;
    }

    SDL_SetWindowPosition(g_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(g_window);

    // GM: Set shader format as only vulkan for now (SPIR-V).
    SDL_GPUShaderFormat shaderFormat = SDL_GPU_SHADERFORMAT_SPIRV;

    g_device = SDL_CreateGPUDevice(shaderFormat, true, "vulkan");
    if (g_device == nullptr)
    {
        printf("Error: SDL_CreateGPUDevice(): %s\n", SDL_GetError());
        return false;
    }
    
    if (!SDL_ClaimWindowForGPUDevice(g_device, g_window))
    {
        printf("Error: SDL_ClaimWindowForGPUDevice(): %s\n", SDL_GetError());
        return false;
    }

    //
    // Setup vertex/index buffers
    //

    SDL_GPUBufferCreateInfo vertexBufferInfo {};
    vertexBufferInfo.size  = sizeof(g_vertices);
    vertexBufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    g_vertexBuffer = SDL_CreateGPUBuffer(g_device, &vertexBufferInfo);
    SDL_SetGPUBufferName(g_device, g_vertexBuffer, "Vertex Buffer");

    SDL_GPUBufferCreateInfo indexBufferInfo {};
    indexBufferInfo.size  = sizeof(g_indices);
    indexBufferInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    g_indexBuffer = SDL_CreateGPUBuffer(g_device, &indexBufferInfo);
    SDL_SetGPUBufferName(g_device, g_indexBuffer, "Index Buffer");

    SDL_GPUTransferBufferCreateInfo vertexTransferInfo {};
    vertexTransferInfo.size  = sizeof(g_vertices);
    vertexTransferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    g_vertexTransferBuffer = SDL_CreateGPUTransferBuffer(g_device, &vertexTransferInfo);

    SDL_GPUTransferBufferCreateInfo indexTransferInfo {};
    indexTransferInfo.size  = sizeof(g_indices);
    indexTransferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    g_indexTransferBuffer = SDL_CreateGPUTransferBuffer(g_device, &indexTransferInfo);

    //
    // Setup shaders
    //

    SDL_GPUShader* vertexShader = Velox::LoadShader("shaders/textured_quad.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX);
    if (vertexShader == nullptr)
    {
        printf("Error: Failed to load vertex shader\n");
        return false;
    }

    SDL_GPUShader* fragmentShader = Velox::LoadShader("shaders/textured_quad.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 1);
    if (vertexShader == nullptr)
    {
        printf("Error: Failed to load fragment shader\n");
        return false;
    }

    //
    // Setup default texture
    //

    SDL_Surface* surface = Velox::LoadImage("assets/textures/missing_texture.png");
    if (surface == nullptr)
        printf("error loading image\n");

    SDL_GPUTextureCreateInfo textureCreateInfo {};
    textureCreateInfo.type   = SDL_GPU_TEXTURETYPE_2D;
    textureCreateInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    textureCreateInfo.width  = surface->w;
    textureCreateInfo.height = surface->h;
    textureCreateInfo.layer_count_or_depth = 1;
    textureCreateInfo.num_levels = 1;
    textureCreateInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

    defaultTexture = SDL_CreateGPUTexture(g_device, &textureCreateInfo);
	SDL_SetGPUTextureName(g_device, defaultTexture, "Missing Texture Texture");

    size_t textureSize = surface->w * surface->h * (sizeof(Uint8) * 4);

    SDL_GPUTransferBufferCreateInfo textureTransferCreateInfo {};
    textureTransferCreateInfo.size  = textureSize;
    textureTransferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    SDL_GPUTransferBuffer* textureTransferBuffer =
        SDL_CreateGPUTransferBuffer(g_device, &textureTransferCreateInfo);

    Uint8* textureTransferPtr = (Uint8*)SDL_MapGPUTransferBuffer(g_device, textureTransferBuffer, false);
	SDL_memcpy(textureTransferPtr, surface->pixels, textureSize);
	SDL_UnmapGPUTransferBuffer(g_device, textureTransferBuffer);

    SDL_GPUTextureTransferInfo textureTransferInfo {};
    textureTransferInfo.transfer_buffer = textureTransferBuffer;
    textureTransferInfo.offset = 0;

    SDL_GPUTextureRegion textureRegion {};
    textureRegion.texture = defaultTexture;
    textureRegion.w = surface->w;
    textureRegion.h = surface->h;
    textureRegion.d = 1;

	SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(g_device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

    SDL_UploadToGPUTexture(copyPass, &textureTransferInfo, &textureRegion, false);

    SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(commandBuffer);

    SDL_DestroySurface(surface);
	SDL_ReleaseGPUTransferBuffer(g_device, textureTransferBuffer);

    //
    // Setup pipeline
    //
    
    SDL_GPUGraphicsPipelineCreateInfo pipelineInfo {};
    // bind shaders
    pipelineInfo.vertex_shader   = vertexShader;
    pipelineInfo.fragment_shader = fragmentShader;
    pipelineInfo.primitive_type  = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

    // describe the vertex buffers
    SDL_GPUVertexBufferDescription vertexBufferDesctiptions[1];
    vertexBufferDesctiptions[0].slot = 0;
    vertexBufferDesctiptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertexBufferDesctiptions[0].instance_step_rate = 0;
    vertexBufferDesctiptions[0].pitch = sizeof(Vertex);
    
    pipelineInfo.vertex_input_state.num_vertex_buffers = 1;
    pipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDesctiptions;

    // describe the vertex attributes
    SDL_GPUVertexAttribute vertexAttributes[3];
    
    // in_position
    vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;  // vec3
    vertexAttributes[0].buffer_slot = 0;
    vertexAttributes[0].location = 0;  // Layout (location = 0) in shader.
    vertexAttributes[0].offset = offsetof(Velox::Vertex, position);

    // in_color
    vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;  // vec4
    vertexAttributes[1].buffer_slot = 0;
    vertexAttributes[1].location = 1;  // Layout (location = 1) in shader.
    vertexAttributes[1].offset = offsetof(Velox::Vertex, color);

    // in_uv
    vertexAttributes[2].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;  // vec2
    vertexAttributes[2].buffer_slot = 0;
    vertexAttributes[2].location = 2;
    vertexAttributes[2].offset = offsetof(Velox::Vertex, uv);

    pipelineInfo.vertex_input_state.num_vertex_attributes = 3;
    pipelineInfo.vertex_input_state.vertex_attributes = vertexAttributes;

    SDL_GPUSamplerCreateInfo samplerCreateInfo {};
    samplerCreateInfo.min_filter = SDL_GPU_FILTER_LINEAR;
	samplerCreateInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
	samplerCreateInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
	samplerCreateInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	samplerCreateInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	samplerCreateInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    g_sampler = SDL_CreateGPUSampler(g_device, &samplerCreateInfo);

    // describe the color target
    SDL_GPUColorTargetDescription colorTargetDescriptions[1];
    colorTargetDescriptions[0] = {};
    colorTargetDescriptions[0].format = SDL_GetGPUSwapchainTextureFormat(g_device, g_window);
    
    pipelineInfo.target_info.num_color_targets = 1;
    pipelineInfo.target_info.color_target_descriptions = colorTargetDescriptions;

    g_graphicsPipeline = SDL_CreateGPUGraphicsPipeline(g_device, &pipelineInfo);
    
    // Release the shaders now, they are now already loaded. 
    SDL_ReleaseGPUShader(g_device, vertexShader);
    SDL_ReleaseGPUShader(g_device, fragmentShader);

    // GM: Add option to change SDL_GPU_PRESENTMODE.
    // MAILBOX is garunteed to be supported.
    SDL_SetGPUSwapchainParameters(g_device, g_window,
        SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
        SDL_GPU_PRESENTMODE_MAILBOX);

    return true;
}

void Velox::StartFrame()
{
    // GM: Not sure if should put the UI ones in UI.cpp or not, here should be fine for now.
    ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    vertexCount = 0;
    indexCount = 0;

    // GM: I don't think we accually need to clear the vertex buffer if we just overwrite vertices.
}

void Velox::EndFrame()
{
    // GM: For now we just insert engine stuff here.
    // Mosly just drawing engine UI elements.
    Velox::DoFrameEndUpdates();

    // GM: Finalises and generates ImGui draw data.
    ImGui::Render();

}

void Velox::DoRenderPass()
{
    ImDrawData* drawData = Velox::GetUIDrawData();       

    // GM: Not sure if we need to check this, I thought when window is minimised then swapchainTexture == nullptr?
    const bool isMinimised = (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f);

    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(g_device);

    SDL_GPUTexture* swapchainTexture;
    SDL_AcquireGPUSwapchainTexture(commandBuffer, g_window, &swapchainTexture, nullptr, nullptr);

    SDL_WaitForGPUIdle(g_device);

    if (swapchainTexture != nullptr && !isMinimised)
    {
        // This is mandatory: call ImGui_ImplSDLGPU3_PrepareDrawData() to upload the vertex/index buffer!
        Imgui_ImplSDLGPU3_PrepareDrawData(drawData, commandBuffer);

        // Copy vertices and indices to the GPU.
        Velox::DoCopyPass(commandBuffer);

        // Setup and start a render pass
        SDL_GPUColorTargetInfo targetInfo = {};
        targetInfo.texture     = swapchainTexture;
        targetInfo.clear_color = CLEAR_COLOR;
        targetInfo.load_op     = SDL_GPU_LOADOP_CLEAR;
        targetInfo.store_op    = SDL_GPU_STOREOP_STORE;
        targetInfo.mip_level   = 0;
        targetInfo.cycle       = false;
        targetInfo.layer_or_depth_plane = 0;

        SDL_GPURenderPass* renderPass = 
            SDL_BeginGPURenderPass(commandBuffer, &targetInfo, 1, nullptr);

        SDL_BindGPUGraphicsPipeline(renderPass, g_graphicsPipeline);

        // bind the vertex buffer
        SDL_GPUBufferBinding vertexBufferBindings[1];
        vertexBufferBindings[0].buffer = g_vertexBuffer;
        vertexBufferBindings[0].offset = 0;
        SDL_BindGPUVertexBuffers(renderPass, 0, vertexBufferBindings, 1);

        SDL_GPUBufferBinding indexBufferBinding {};
        indexBufferBinding.buffer = g_indexBuffer;
        indexBufferBinding.offset = 0;
        SDL_BindGPUIndexBuffer(renderPass, &indexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

        SDL_GPUTextureSamplerBinding textureSamplerBinding {};
        textureSamplerBinding.texture = defaultTexture;
        textureSamplerBinding.sampler = g_sampler;
        SDL_BindGPUFragmentSamplers(renderPass, 0, &textureSamplerBinding, 1);

        SDL_DrawGPUIndexedPrimitives(renderPass, indexCount, 1, 0, 0, 0);

        // Render ImGui stuff.
        ImGui_ImplSDLGPU3_RenderDrawData(drawData, commandBuffer, renderPass);

        SDL_EndGPURenderPass(renderPass);
    }

    SDL_SubmitGPUCommandBuffer(commandBuffer);
}

void Velox::DoCopyPass(SDL_GPUCommandBuffer* commandBuffer)
{
    Vertex* vertexData = (Vertex*)SDL_MapGPUTransferBuffer(g_device, g_vertexTransferBuffer, true);
    Uint32* indexData  = (Uint32*)SDL_MapGPUTransferBuffer(g_device, g_indexTransferBuffer,  true);

    // Copy data to transfer buffer.
    SDL_memcpy(vertexData, g_vertices, sizeof(g_vertices));
    SDL_memcpy(indexData,  g_indices,  sizeof(g_indices));

    SDL_UnmapGPUTransferBuffer(g_device, g_vertexTransferBuffer);
    SDL_UnmapGPUTransferBuffer(g_device, g_indexTransferBuffer);

    SDL_GPUTransferBufferLocation vertexLocation {};
    vertexLocation.transfer_buffer = g_vertexTransferBuffer;
    vertexLocation.offset = 0;

    SDL_GPUTransferBufferLocation indexLocation {};
    indexLocation.transfer_buffer = g_indexTransferBuffer;
    indexLocation.offset = 0;

    SDL_GPUBufferRegion vertexRegion {};
    vertexRegion.buffer = g_vertexBuffer;
    vertexRegion.size   = sizeof(g_vertices);
    vertexRegion.offset = 0;

    SDL_GPUBufferRegion indexRegion {};
    indexRegion.buffer = g_indexBuffer;
    indexRegion.size   = sizeof(g_indices);
    indexRegion.offset = 0;

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

    SDL_UploadToGPUBuffer(copyPass, &vertexLocation, &vertexRegion, true);
    SDL_UploadToGPUBuffer(copyPass, &indexLocation,  &indexRegion,  true);

    SDL_EndGPUCopyPass(copyPass);
}

// TODO: Verify if these need to be in the right order.
void Velox::DeInitRenderer()
{
    SDL_WaitForGPUIdle(g_device);

    SDL_ReleaseGPUSampler(g_device, g_sampler);
    SDL_ReleaseGPUTexture(g_device, defaultTexture);

    SDL_ReleaseGPUBuffer(g_device, g_vertexBuffer);
    SDL_ReleaseGPUBuffer(g_device, g_indexBuffer);

    SDL_ReleaseGPUTransferBuffer(g_device, g_vertexTransferBuffer);
    SDL_ReleaseGPUTransferBuffer(g_device, g_indexTransferBuffer);

    SDL_ReleaseGPUGraphicsPipeline(g_device, g_graphicsPipeline);

    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLGPU3_Shutdown();

    SDL_ReleaseWindowFromGPUDevice(g_device, g_window);
    SDL_DestroyGPUDevice(g_device);
    SDL_DestroyWindow(g_window);
}

// filepath relative to the exe (in build/bin).
SDL_GPUShader* Velox::LoadShader(const char* filepath, SDL_GPUShaderStage shaderStage, int numSamplers, int numUniforms)
{
    char absoluteFilepath[1024], *ptr;
    
    SDL_strlcpy(absoluteFilepath, SDL_GetBasePath(), sizeof(absoluteFilepath));
    SDL_strlcat(absoluteFilepath, filepath, sizeof(absoluteFilepath));
    
    size_t shaderCodeSize;
    void* shaderCode = SDL_LoadFile(absoluteFilepath, &shaderCodeSize);
    if (shaderCode == nullptr)
    {
        printf("Error: SDL_LoadFile(): %s\n", SDL_GetError());
        return nullptr;
    }

    SDL_GPUShaderCreateInfo shaderInfo {};
    shaderInfo.code       = (Uint8*)shaderCode; // convert to an array of bytes
    shaderInfo.code_size  = shaderCodeSize;
    shaderInfo.entrypoint = "main";
    shaderInfo.format     = SDL_GPU_SHADERFORMAT_SPIRV; // loading .spv shaders
    shaderInfo.stage      = shaderStage;
                                           
    shaderInfo.num_samplers         = numSamplers;
    shaderInfo.num_storage_buffers  = 0;
    shaderInfo.num_storage_textures = 0;
    shaderInfo.num_uniform_buffers  = numUniforms;

    SDL_GPUShader* vertexShader = SDL_CreateGPUShader(g_device, &shaderInfo);

    // Free the file
    SDL_free(shaderCode);

    return vertexShader;
}

SDL_Surface* Velox::LoadImage(const char* filepath)
{
    char absoluteFilepath[1024], *ptr;
    
    int frame = 0;
    SDL_strlcpy(absoluteFilepath, SDL_GetBasePath(), sizeof(absoluteFilepath));
    SDL_strlcat(absoluteFilepath, filepath,          sizeof(absoluteFilepath));
    
    // printf("Filepath: %s\n", absoluteFilepath);

    SDL_Surface* surface = IMG_Load(absoluteFilepath);
    if (surface == nullptr)
    {
        printf("Error: Failed to load image from filepath: %s\n", absoluteFilepath);
        return nullptr;
    }

    if (surface->format != SDL_PIXELFORMAT_ABGR8888)
    {
        printf("Error: Loaded image has wrong pixel format. Expected \"SDL_PIXELFORMAT_ABGR888\", \
            found: \"%s\"\n", SDL_GetPixelFormatName(surface->format));

        // This is safe for now.
        // TODO: Atempt to convert maybe?

        return nullptr;
    }

    return surface;
}

Uint32 Velox::AddVertex(Velox::Vertex vertex)
{
    int index = vertexCount;

    g_vertices[vertexCount] = vertex;
    vertexCount++;

    return index;
}

void Velox::AddIndex(Uint32 index)
{
    g_indices[indexCount] = index;
    indexCount++;
}

