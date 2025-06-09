#pragma once

#include "Core.h"

#include <optional>
#include <vector>
#include <array>

#include <SDL3/SDL_events.h>
#include <vulkan/vulkan_core.h>

struct SDL_Window;

namespace Velox {

struct Arena;

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct UniformBufferObject {
    ivec2 screenResolution;
};

struct Vertex {
    vec3 position;
    vec4 color;
    vec2 uv;
    int32_t textureIndex;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription {};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, uv);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32_SINT;
        attributeDescriptions[3].offset = offsetof(Vertex, textureIndex);

        return attributeDescriptions;
    }
};

struct Texture2D { 
    VkImage        image;
    VkDeviceMemory imageMemory;
    VkImageView    imageView;
    VkSampler      sampler;
};

SDL_Window* GetWindow();

VkDevice* GetDevice();

ivec2 GetWindowSize();

float GetDisplayScale();

bool InitRenderer();

void CreateInstance();

bool CheckValidationLayerSupport();

void PickPhysicalDevice();
bool CheckPhysicalDeviceExtensionSupport(VkPhysicalDevice device);
bool IsDeviceSuitable(VkPhysicalDevice device);
int RateDeviceSuitability(VkPhysicalDevice device);

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

void CreateLogicalDevice();

SwapchainSupportDetails GetSwapchainSupportDetails(VkPhysicalDevice device);

void CreateSwapchain();
VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

void ReCreateSwapchain();

void CleanupSwapchain();

void CreateImageViews();

void CreateRenderPass();

void CreateDiscriptorSetLayout();

void CreateGraphicsPipeline();

void CreateFrameBuffers();

void CreateCommandPool();

void CreateDescriptorPool();

void CreateDescriptorSets();

void UpdateTextureDescriptors();

void CreateCommandBuffers();

void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
        VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void CreateVertexBuffer();

void CreateIndexBuffer();

void CreateUniformBuffers();

void UpdateUniformBuffer(uint32_t imageIndex);

uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

void CreateSyncObjects();

bool ForwardSDLEventToRenderer(SDL_Event* event);

void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    
VkCommandBuffer BeginSingleTimeCommands();

void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

void StartFrame();

void DrawFrame();

void EndFrame();

void DoRenderPass();

void DoCopyPass();

void CopyVerticesToGPU();
void CopyIndicesToGPU();

void DeInitRenderer();

void LoadShader(VkShaderModule* shaderModule, const char* filepath, Velox::Arena* allocator);

void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usageFlags, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

VkCommandBuffer BeginSingleTimeCommands();

void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

void LoadImage(const char* filepath, int index);

int LoadTextureInternal(const char* filepath);

void CreateImageView(int index, VkFormat format);

void CreateTextureSampler(int index);

// Returns the index of the vertex.
uint32_t AddVertex(Vertex vertex);

void AddIndex(uint32_t index);


}
