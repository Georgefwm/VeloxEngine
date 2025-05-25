#include "Renderer.h"

#include "SDL3/SDL_filesystem.h"
#include "UI.h"

#include <glm/glm.hpp>
#include <SDL3/SDL_gpu.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include <cstdio>

constexpr size_t VERTEX_BUFFER_SIZE = 512;
constexpr size_t INDEX_BUFFER_SIZE  = 512;

constexpr SDL_FColor CLEAR_COLOR = {0.5, 0.5, 0.5, 1.0 };

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

    SDL_GPUBufferCreateInfo vertexBufferInfo {};
    vertexBufferInfo.size  = sizeof(g_vertices);
    vertexBufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    g_vertexBuffer = SDL_CreateGPUBuffer(g_device, &vertexBufferInfo);

    SDL_GPUBufferCreateInfo indexBufferInfo {};
    indexBufferInfo.size  = sizeof(g_indices);
    indexBufferInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    g_indexBuffer = SDL_CreateGPUBuffer(g_device, &indexBufferInfo);

    SDL_GPUTransferBufferCreateInfo vertexTransferInfo {};
    vertexTransferInfo.size  = sizeof(g_vertices);
    vertexTransferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    g_vertexTransferBuffer = SDL_CreateGPUTransferBuffer(g_device, &vertexTransferInfo);

    SDL_GPUTransferBufferCreateInfo indexTransferInfo {};
    indexTransferInfo.size  = sizeof(g_indices);
    indexTransferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    g_indexTransferBuffer = SDL_CreateGPUTransferBuffer(g_device, &indexTransferInfo);

    SDL_GPUShader* vertexShader =
        Velox::LoadShader("shaders\\vertex_base.spv", SDL_GPU_SHADERSTAGE_VERTEX);

    if (vertexShader == nullptr)
    {
        printf("Error: Failed to load vertex shader\n");
        return false;
    }

    SDL_GPUShader* fragmentShader =
        Velox::LoadShader("shaders\\fragment_base.spv", SDL_GPU_SHADERSTAGE_FRAGMENT);

    if (vertexShader == nullptr)
    {
        printf("Error: Failed to load fragment shader\n");
        return false;
    }

    SDL_GPUGraphicsPipelineCreateInfo pipelineInfo {};
    // bind shaders
    pipelineInfo.vertex_shader   = vertexShader;
    pipelineInfo.fragment_shader = fragmentShader;
    pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

    // describe the vertex buffers
    SDL_GPUVertexBufferDescription vertexBufferDesctiptions[1];
    vertexBufferDesctiptions[0].slot = 0;
    vertexBufferDesctiptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertexBufferDesctiptions[0].instance_step_rate = 0;
    vertexBufferDesctiptions[0].pitch = sizeof(Vertex);
    
    pipelineInfo.vertex_input_state.num_vertex_buffers = 1;
    pipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDesctiptions;

    // describe the vertex attribute
    SDL_GPUVertexAttribute vertexAttributes[2];
    
    // in_position
    vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;  // vec3
    vertexAttributes[0].buffer_slot = 0;  // Fetch data from the buffer at slot 0.
    vertexAttributes[0].location = 0;     // Layout (location = 0) in shader.
    vertexAttributes[0].offset = 0;       // start from the first byte from current buffer position
    
    // in_color
    vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4; //vec4
    vertexAttributes[1].buffer_slot = 0;             // use buffer at slot 0
    vertexAttributes[1].location = 1;                // layout (location = 1) in shader
    vertexAttributes[1].offset = sizeof(float) * 3;  // 4th float from current buffer position

    pipelineInfo.vertex_input_state.num_vertex_attributes = 2;
    pipelineInfo.vertex_input_state.vertex_attributes = vertexAttributes;

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

        SDL_GPUBufferBinding indexBufferBinding {};
        indexBufferBinding.buffer = g_indexBuffer;
        indexBufferBinding.offset = 0;

        SDL_BindGPUVertexBuffers(renderPass, 0, vertexBufferBindings, 1);
        SDL_BindGPUIndexBuffer(renderPass, &indexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_32BIT);

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
SDL_GPUShader* Velox::LoadShader(const char* filepath, SDL_GPUShaderStage shaderStage)
{
    char absoluteFilepath[1024], *ptr;
    
    int frame = 0;
    SDL_strlcpy(absoluteFilepath, SDL_GetBasePath(), sizeof(absoluteFilepath));
    SDL_strlcat(absoluteFilepath, filepath, sizeof(absoluteFilepath));
    
    bool fileFound = SDL_GetPathInfo(absoluteFilepath, nullptr);
    if (!fileFound)
    {
        printf("ERROR: Could not find shader file at path: %s\n.", absoluteFilepath);
        return nullptr;
    }

    size_t shaderCodeSize;
    void* shaderCode = SDL_LoadFile(absoluteFilepath, &shaderCodeSize);
    if (shaderCode == nullptr)
    {
        printf("Error: SDL_LoadFile(): %s\n", SDL_GetError());
        return nullptr;
    }

    SDL_GPUShaderCreateInfo shaderInfo {};
    shaderInfo.code       = (Uint8*)shaderCode; //convert to an array of bytes
    shaderInfo.code_size  = shaderCodeSize;
    shaderInfo.entrypoint = "main";
    shaderInfo.format     = SDL_GPU_SHADERFORMAT_SPIRV; // loading .spv shaders
    shaderInfo.stage      = shaderStage; // vertex shader
                                           
    shaderInfo.num_samplers         = 0;
    shaderInfo.num_storage_buffers  = 0;
    shaderInfo.num_storage_textures = 0;
    shaderInfo.num_uniform_buffers  = 0;

    SDL_GPUShader* vertexShader = SDL_CreateGPUShader(g_device, &shaderInfo);

    // Free the file
    SDL_free(shaderCode);

    return vertexShader;
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

