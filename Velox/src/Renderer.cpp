#include "Primitive.h"
#ifndef VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#define VK_EXT_DEBUG_REPORT_EXTENSION_NAME "VK_EXT_debug_report"
#endif

#include "Renderer.h"

#include "Arena.h"

#include <glm/glm.hpp>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3_image/SDL_image.h>
#include <imgui_impl_sdl3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_enum_string_helper.h> 

#include <cstddef>
#include <cstdio>
#include <fstream>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

#ifdef NDEBUG
    constexpr bool enableValidationLayers = false;
#else
    constexpr bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

const std::vector<const char*> requiredPhysicalDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

constexpr size_t VERTEX_BUFFER_SIZE = 512;
constexpr size_t INDEX_BUFFER_SIZE  = 512;
constexpr size_t MAX_TEXTURE_COUNT  = 5;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;


SDL_Window* g_window;

VkInstance g_instance;

VkPhysicalDevice g_physicalDevice = VK_NULL_HANDLE;
VkDevice g_device;

VkSurfaceKHR g_surface;

VkQueue g_graphicsQueue;
VkQueue g_presentQueue;

VkSwapchainKHR g_swapchain;
std::vector<VkImage> g_swapchainImages;
std::vector<VkImageView> g_swapchainImageViews;

VkFormat g_swapchainImageFormat;
VkExtent2D g_swapchainExtent;

VkRenderPass g_renderPass;

VkDescriptorSetLayout g_descriptorSetLayout;
VkPipelineLayout g_pipelineLayout;
VkPipeline g_graphicsPipeline;

std::vector<VkFramebuffer> g_swapchainFrameBuffers;

VkCommandPool g_commandPool;

VkDescriptorPool g_descriptorPool;
std::vector<VkDescriptorSet> g_descriptorSets;

std::vector<VkCommandBuffer> g_commandBuffers;
std::vector<VkSemaphore> g_imageAvailableSemaphores;
std::vector<VkSemaphore> g_renderFinishedSemaphores;
std::vector<VkFence> g_inFlightFences;

uint32_t g_currentFrame = 0;
bool g_frameBufferResized = false;

VkBuffer g_vertexBuffer;
VkDeviceMemory g_vertexBufferMemory;

VkBuffer g_indexBuffer;
VkDeviceMemory g_indexBufferMemory;

std::vector<VkBuffer> g_uniformBuffers;
std::vector<VkDeviceMemory> g_uniformBuffersMemory;
std::vector<void*> g_uniformBuffersMapped;

uint32_t g_vertexCount = 0;
Velox::Vertex g_vertices[VERTEX_BUFFER_SIZE];

uint32_t g_indexCount = 0;
uint32_t g_indices[INDEX_BUFFER_SIZE];

SDL_Window* Velox::GetWindow() { return g_window; }
VkDevice*   Velox::GetDevice() { return &g_device; }

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
    if (enableValidationLayers && !CheckValidationLayerSupport())
        throw std::runtime_error("validation layers requested, but not available!");

    // Velox::Arena tempData(100000);

    SDL_WindowFlags windowFlags;
    windowFlags |= SDL_WINDOW_VULKAN;
    windowFlags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
    
    g_window = SDL_CreateWindow("Velox App", 1920, 1080, windowFlags);
    if (g_window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        throw std::runtime_error("Failed to create SDL window");
    }

    SDL_SetWindowPosition(g_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    // GM: Maybe move to bottom of init... dunno yet.
    SDL_ShowWindow(g_window);

    Velox::CreateInstance();
    
    SDL_Vulkan_CreateSurface(g_window, g_instance, nullptr, &g_surface);
    if (g_surface == nullptr)
    {
        printf("Error: SDL_Vulkan_CreateSurface(): %s\n", SDL_GetError());
        throw std::runtime_error("Failed to create Vulkan surface");
    }

    Velox::PickPhysicalDevice();

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(g_physicalDevice, &deviceProperties);
    printf("Using physical device: %s\n", deviceProperties.deviceName);

    Velox::CreateLogicalDevice();

    Velox::CreateSwapchain();
    Velox::CreateImageViews();

    Velox::CreateRenderPass();

    Velox::CreateDiscriptorSetLayout();

    Velox::CreateGraphicsPipeline();

    Velox::CreateFrameBuffers();

    Velox::CreateCommandPool();

    Velox::CreateVertexBuffer();
    Velox::CreateIndexBuffer();
    Velox::CreateUniformBuffers();

    Velox::CreateDescriptorPool();
    Velox::CreateDescriptorSets();

    Velox::CreateCommandBuffers();

    Velox::CreateSyncObjects();

    return true;
}

void Velox::CreateInstance()
{
    uint32_t instanceExtensionCount;
    const char * const *instance_extensions = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount);
    
    if (instance_extensions == NULL)
        throw std::runtime_error("Failed to get supported SDL instance extensions");
    
    // GM: Add one as we manually add the VK_EXT_DEBUG extension below.
    int countExtensions = instanceExtensionCount + 1;

    const char** extensions = reinterpret_cast<const char**>(SDL_malloc(countExtensions * sizeof(const char*)));
    extensions[0] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
    SDL_memcpy(&extensions[1], instance_extensions, instanceExtensionCount * sizeof(const char*));

    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Test Vulkan App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Velox Engine",
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0),
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo InstanceCreateInfo {};
    InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    InstanceCreateInfo.pApplicationInfo = &appInfo;
    InstanceCreateInfo.enabledExtensionCount = countExtensions;
    InstanceCreateInfo.ppEnabledExtensionNames = extensions;
    InstanceCreateInfo.enabledLayerCount = 0;

    if (enableValidationLayers)
    {
        InstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        InstanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    }

    VkResult result = vkCreateInstance(&InstanceCreateInfo, nullptr, &g_instance);
    if (result != VK_SUCCESS)
    {
        printf("ERROR: vkCreateInstance(): %s\n", string_VkResult(result));
        throw std::runtime_error("Failed to create Vulkan instance");
    }

    SDL_free(extensions);
}

bool Velox::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;
    
        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}

void Velox::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(g_instance, &deviceCount, nullptr);

    if (deviceCount == 0)
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(g_instance, &deviceCount, devices.data());

    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto& device : devices)
    {
        if (!Velox::IsDeviceSuitable(device))
            continue;

        int score = Velox::RateDeviceSuitability(device);
        candidates.insert(std::make_pair(score, device));
    }

    // Check if the best candidate is suitable at all.
    if (candidates.rbegin()->first > 0)
        g_physicalDevice = candidates.rbegin()->second;
    else
        throw std::runtime_error("Failed to find a suitable GPU");
}

bool Velox::CheckPhysicalDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(requiredPhysicalDeviceExtensions.begin(), 
            requiredPhysicalDeviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool Velox::IsDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = Velox::FindQueueFamilies(device);

    bool extensionsSupported = Velox::CheckPhysicalDeviceExtensionSupport(device);

    bool swapchainAdequate = false;
    if (extensionsSupported) {
        SwapchainSupportDetails swapchainSupport = Velox::GetSwapchainSupportDetails(device);
        swapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
    }

    return indices.graphicsFamily.has_value() && extensionsSupported && swapchainAdequate;
}

int Velox::RateDeviceSuitability(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    int score = 0;

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 1000;

    score += deviceProperties.limits.maxImageDimension2D;

    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader)
        return 0;

    return score;
}

Velox::QueueFamilyIndices Velox::FindQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    
    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, g_surface, &presentSupport);

        if (presentSupport)
            indices.presentFamily = i;

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        if (indices.IsComplete())
            break;

        i++;
    }

    return indices;
}

void Velox::CreateLogicalDevice()
{
    QueueFamilyIndices indices = Velox::FindQueueFamilies(g_physicalDevice);
    
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(), 
        indices.presentFamily.value()
    };
    
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    VkPhysicalDeviceFeatures deviceFeatures {};

    VkDeviceCreateInfo deviceCreateInfo {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos    = queueCreateInfos.data();
    deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredPhysicalDeviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = requiredPhysicalDeviceExtensions.data();

    VkResult result = vkCreateDevice(g_physicalDevice, &deviceCreateInfo, nullptr, &g_device);
    if (result != VK_SUCCESS)
    {
        printf("ERROR: vkCreateDevice(): %s\n", string_VkResult(result));
        throw std::runtime_error("Failed to create Vulkan logical device");
    }

    vkGetDeviceQueue(g_device, indices.graphicsFamily.value(), 0, &g_graphicsQueue);
    vkGetDeviceQueue(g_device, indices.presentFamily.value(),  0, &g_presentQueue);
}

Velox::SwapchainSupportDetails Velox::GetSwapchainSupportDetails(VkPhysicalDevice device)
{
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, g_surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, g_surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, g_surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, g_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, g_surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

void Velox::CreateSwapchain()
{
    SwapchainSupportDetails swapchainSupport = Velox::GetSwapchainSupportDetails(g_physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = Velox::ChooseSwapSurfaceFormat(swapchainSupport.formats);
    VkPresentModeKHR presentMode = Velox::ChooseSwapPresentMode(swapchainSupport.presentModes);
    VkExtent2D extent = Velox::ChooseSwapExtent(swapchainSupport.capabilities);

    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;

    // Check to make sure we don't exceed maximum image count supported by the GPU.
    if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
        imageCount = swapchainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR swapchainCreateInfo {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = g_surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = Velox::FindQueueFamilies(g_physicalDevice);
    uint32_t queueFamilyIndices[] = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };

    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (indices.graphicsFamily != indices.presentFamily) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    swapchainCreateInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(g_device, &swapchainCreateInfo, nullptr, &g_swapchain);
    if (result != VK_SUCCESS)
    {
        printf("ERROR: vkCreateSwapchainKHR(): %s\n", string_VkResult(result));
        throw std::runtime_error("Failed to create swapchain");
    }

    vkGetSwapchainImagesKHR(g_device, g_swapchain, &imageCount, nullptr);
    g_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(g_device, g_swapchain, &imageCount, g_swapchainImages.data());

    g_swapchainImageFormat = surfaceFormat.format;
    g_swapchainExtent = extent;
}

VkSurfaceFormatKHR Velox::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && 
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;
    }

    return availableFormats[0];
}

VkPresentModeKHR Velox::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    // Try to use prefered mode if its available (MAILBOX).
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    // Garunteed to be supported.
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Velox::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;

    int width, height;
    SDL_GetWindowSizeInPixels(g_window, &width, &height);

    VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

    actualExtent.width = glm::clamp(actualExtent.width,
            capabilities.minImageExtent.width, capabilities.maxImageExtent.width);

    actualExtent.height = glm::clamp(actualExtent.height,
            capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}

void Velox::ReCreateSwapchain()
{
    vkDeviceWaitIdle(g_device);

    Velox::CleanupSwapchain();

    Velox::CreateSwapchain();
    Velox::CreateImageViews();
    Velox::CreateFrameBuffers();
}

void Velox::CleanupSwapchain()
{
    for (size_t i = 0; i < g_swapchainFrameBuffers.size(); i++) {
        vkDestroyFramebuffer(g_device, g_swapchainFrameBuffers[i], nullptr);
    }

    for (size_t i = 0; i < g_swapchainImageViews.size(); i++) {
        vkDestroyImageView(g_device, g_swapchainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(g_device, g_swapchain, nullptr);
}

void Velox::CreateImageViews()
{
    g_swapchainImageViews.resize(g_swapchainImages.size());

    for (size_t i = 0; i < g_swapchainImages.size(); i++) {
        VkImageViewCreateInfo imageViewCreateInfo {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = g_swapchainImages[i];

        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = g_swapchainImageFormat;

        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView(g_device, &imageViewCreateInfo, nullptr, &g_swapchainImageViews[i]);
        if (result != VK_SUCCESS)
        {
            printf("ERROR: vkCreateImageView(): %s\n", string_VkResult(result));
            throw std::runtime_error("Failed to create Vulkan Image View");
        }
    }
}

void Velox::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment {};
    colorAttachment.format = g_swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachment;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &dependency;

    VkResult result = vkCreateRenderPass(g_device, &renderPassCreateInfo, nullptr, &g_renderPass);
    if (result != VK_SUCCESS)
    {
        printf("ERROR: vkCreateRenderPass(): %s\n", string_VkResult(result));
        throw std::runtime_error("Failed to create Vulkan Render Pass");
    }
}

void Velox::CreateDiscriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = 1;
    layoutCreateInfo.pBindings = &uboLayoutBinding;

    VkResult result = vkCreateDescriptorSetLayout(g_device, &layoutCreateInfo, nullptr, &g_descriptorSetLayout);
    if (result != VK_SUCCESS)
    {
        printf("ERROR: vkCreateDiscriptorSetLayout(): %s\n", string_VkResult(result));
        throw std::runtime_error("Failed to create discriptor set layout");
    }
}

void Velox::CreateGraphicsPipeline()
{
    Velox::Arena tempData(100000);

    VkShaderModule vertShaderModule;
    Velox::LoadShader(&vertShaderModule, "indexed_textures.vert.spv", &tempData);

    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo {};
    vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCreateInfo.module = vertShaderModule;
    vertShaderStageCreateInfo.pName = "main";

    VkShaderModule fragShaderModule;
    Velox::LoadShader(&fragShaderModule, "indexed_textures.frag.spv", &tempData);

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo {};
    fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageCreateInfo.module = fragShaderModule;
    fragShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertShaderStageCreateInfo, 
        fragShaderStageCreateInfo,
    };

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo {};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

    auto bindingDescription    = Velox::Vertex::GetBindingDescription();
    auto attributeDescriptions = Velox::Vertex::GetAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo {};
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo {};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo {};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.scissorCount  = 1;

    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo {};
    rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerCreateInfo.depthClampEnable = VK_FALSE;
    rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizerCreateInfo.lineWidth = 1.0f;
    rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizerCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizerCreateInfo.depthBiasConstantFactor = 0.0f; // Optional
    rasterizerCreateInfo.depthBiasClamp = 0.0f; // Optional
    rasterizerCreateInfo.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo {};
    multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
    multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisamplingCreateInfo.minSampleShading = 1.0f; // Optional
    multisamplingCreateInfo.pSampleMask = nullptr; // Optional
    multisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
    multisamplingCreateInfo.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment {};
    colorBlendAttachment.colorWriteMask = 
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;             // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;             // Optional

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo {};
    colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = &colorBlendAttachment;
    colorBlendStateCreateInfo.blendConstants[0] = 0.0f; // Optional
    colorBlendStateCreateInfo.blendConstants[1] = 0.0f; // Optional
    colorBlendStateCreateInfo.blendConstants[2] = 0.0f; // Optional
    colorBlendStateCreateInfo.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &g_descriptorSetLayout;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;    // Optional
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr; // Optional

    VkResult result = vkCreatePipelineLayout(g_device, &pipelineLayoutCreateInfo, nullptr, &g_pipelineLayout);
    if (result != VK_SUCCESS)
    {
        printf("ERROR: vkCreatePipelineLayout(): %s\n", string_VkResult(result));
        throw std::runtime_error("Failed to create Vulkan Pipeline Layout");
    }

    VkGraphicsPipelineCreateInfo pipelineCreateInfo {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState   = &vertexInputCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    pipelineCreateInfo.pViewportState      = &viewportStateCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
    pipelineCreateInfo.pMultisampleState   = &multisamplingCreateInfo;
    pipelineCreateInfo.pDepthStencilState  = nullptr; // Optional
    pipelineCreateInfo.pColorBlendState    = &colorBlendStateCreateInfo;
    pipelineCreateInfo.pDynamicState       = &dynamicStateCreateInfo;
    pipelineCreateInfo.layout = g_pipelineLayout;
    pipelineCreateInfo.renderPass = g_renderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineCreateInfo.basePipelineIndex = -1; // Optional

    result = vkCreateGraphicsPipelines(g_device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &g_graphicsPipeline);
    if (result != VK_SUCCESS)
    {
        printf("ERROR: vkCreateGraphicsPipelines(): %s\n", string_VkResult(result));
        throw std::runtime_error("Failed to create Vulkan Graphics Pipeline");
    }

    vkDestroyShaderModule(g_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(g_device, vertShaderModule, nullptr);
}

void Velox::CreateFrameBuffers()
{
    g_swapchainFrameBuffers.resize(g_swapchainImageViews.size());

    for (size_t i = 0; i < g_swapchainImageViews.size(); i++)
    {
        VkImageView attachments[] = {
            g_swapchainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = g_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = g_swapchainExtent.width;
        framebufferInfo.height = g_swapchainExtent.height;
        framebufferInfo.layers = 1;

        VkResult result = vkCreateFramebuffer(g_device, &framebufferInfo, nullptr, &g_swapchainFrameBuffers[i]);
        if (result != VK_SUCCESS)
        {
            printf("ERROR: vkCreateFrameBuffer(%zu): %s\n", i, string_VkResult(result));
            throw std::runtime_error("Failed to create Vulkan Frame Buffer");
        }
    }
}

void Velox::CreateCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = Velox::FindQueueFamilies(g_physicalDevice);

    VkCommandPoolCreateInfo poolCreateInfo {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VkResult result = vkCreateCommandPool(g_device, &poolCreateInfo, nullptr, &g_commandPool);
    if (result != VK_SUCCESS)
    {
        printf("ERROR: vkCreateFrameBuffer(): %s\n", string_VkResult(result));
        throw std::runtime_error("Failed to create command pool");
    }
}

void Velox::CreateDescriptorPool()
{
    VkDescriptorPoolSize poolSize {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolCreateInfo {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.poolSizeCount = 1;
    poolCreateInfo.pPoolSizes = &poolSize;
    poolCreateInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    
    VkResult result = vkCreateDescriptorPool(g_device, &poolCreateInfo, nullptr, &g_descriptorPool);
    if (result != VK_SUCCESS)
    {
        printf("ERROR: vkCreateDescriptorPool(): %s\n", string_VkResult(result));
        throw std::runtime_error("Failed to create descriptor pool");
    }
}

void Velox::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, g_descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = g_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    g_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

    VkResult result = vkAllocateDescriptorSets(g_device, &allocInfo, g_descriptorSets.data());
    if (result != VK_SUCCESS)
    {
        printf("ERROR: vkAllocateDescriptorSets(): %s\n", string_VkResult(result));
        throw std::runtime_error("Failed to allocate descriptor sets");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo {};
        bufferInfo.buffer = g_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = g_descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional
        
        vkUpdateDescriptorSets(g_device, 1, &descriptorWrite, 0, nullptr);
    }

}

void Velox::CreateCommandBuffers()
{
    g_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo commandBufferAllocInfo {};
    commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocInfo.commandPool = g_commandPool;
    commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocInfo.commandBufferCount = (uint32_t)g_commandBuffers.size();

    VkResult result = vkAllocateCommandBuffers(g_device, &commandBufferAllocInfo, g_commandBuffers.data());
    if (result != VK_SUCCESS)
    {
        printf("ERROR: vkAllocateCommandBuffers(): %s\n", string_VkResult(result));
        throw std::runtime_error("Failed to create command buffers");
    }
}

void Velox::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo commandBufferBeginInfo {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = 0; // Optional
    commandBufferBeginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassBeginInfo {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = g_renderPass;
    renderPassBeginInfo.framebuffer = g_swapchainFrameBuffers[imageIndex];
    renderPassBeginInfo.renderArea.offset = { 0, 0 };
    renderPassBeginInfo.renderArea.extent = g_swapchainExtent;

    VkClearValue clearColor = {{{ 0.0f, 0.0f, 0.0f, 1.0f }}};
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_graphicsPipeline);

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width  = (float) g_swapchainExtent.width;
    viewport.height = (float) g_swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor {};
    scissor.offset = { 0, 0 };
    scissor.extent = g_swapchainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = { g_vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, g_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
            g_pipelineLayout, 0, 1, &g_descriptorSets[g_currentFrame], 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, g_indexCount, 1, 0, 0, 1);

    vkCmdEndRenderPass(commandBuffer);

    VkResult result = vkEndCommandBuffer(commandBuffer);
    if (result != VK_SUCCESS)
    {
        printf("ERROR: vkEndCommandBuffer(): %s\n", string_VkResult(result));
        throw std::runtime_error("Errors occurred while proccessing command buffer");
    }
}

void Velox::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(g_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        throw std::runtime_error("failed to create buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(g_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = Velox::FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(g_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate buffer memory!");

    vkBindBufferMemory(g_device, buffer, bufferMemory, 0);
}

void Velox::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = g_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(g_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion {};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(g_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(g_graphicsQueue);

    vkFreeCommandBuffers(g_device, g_commandPool, 1, &commandBuffer);
}

void Velox::CreateVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(g_vertices);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    Velox::CreateBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void* data;
    vkMapMemory(g_device, stagingBufferMemory, 0, bufferSize, 0, &data);

    memcpy(data, g_vertices, (size_t)bufferSize);

    vkUnmapMemory(g_device, stagingBufferMemory);

    Velox::CreateBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        g_vertexBuffer,
        g_vertexBufferMemory);

    Velox::CopyBuffer(stagingBuffer, g_vertexBuffer, bufferSize);

    vkDestroyBuffer(g_device, stagingBuffer, nullptr);
    vkFreeMemory(g_device, stagingBufferMemory, nullptr);
}

void Velox::CreateIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(g_indices);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    Velox::CreateBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void* data;
    vkMapMemory(g_device, stagingBufferMemory, 0, bufferSize, 0, &data);

    memcpy(data, g_indices, (size_t) bufferSize);

    vkUnmapMemory(g_device, stagingBufferMemory);

    Velox::CreateBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        g_indexBuffer,
        g_indexBufferMemory);

    Velox::CopyBuffer(stagingBuffer, g_indexBuffer, bufferSize);

    vkDestroyBuffer(g_device, stagingBuffer, nullptr);
    vkFreeMemory(g_device, stagingBufferMemory, nullptr);
}

void Velox::CreateUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(Velox::UniformBufferObject);

    g_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    g_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    g_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        Velox::CreateBuffer(
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            g_uniformBuffers[i],
            g_uniformBuffersMemory[i]);

        vkMapMemory(g_device, g_uniformBuffersMemory[i], 0, bufferSize, 0, &g_uniformBuffersMapped[i]);
    }
}

void Velox::UpdateUniformBuffer(uint32_t imageIndex)
{
    UniformBufferObject ubo {};
    ubo.screenResolution = { 0, 0 };

    SDL_GetWindowSize(g_window, &ubo.screenResolution.x, &ubo.screenResolution.y);

    memcpy(g_uniformBuffersMapped[imageIndex], &ubo, sizeof(ubo));
}

uint32_t Velox::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(g_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    throw std::runtime_error("failed to find suitable memory type");
}

void Velox::CreateSyncObjects()
{
    g_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    g_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    g_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreCreateInfo {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkResult result = vkCreateSemaphore(g_device, &semaphoreCreateInfo,  nullptr, &g_imageAvailableSemaphores[i]);
        if (result != VK_SUCCESS)
            throw std::runtime_error("Error creating imageAvailable semphore");

        result = vkCreateSemaphore(g_device, &semaphoreCreateInfo,  nullptr, &g_renderFinishedSemaphores[i]);
        if (result != VK_SUCCESS)
            throw std::runtime_error("Error creating renderFinished semphore");
    }

    VkFenceCreateInfo fenceCreateInfo {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Prevent waiting for first frame.

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkResult result = vkCreateFence(g_device, &fenceCreateInfo,  nullptr, &g_inFlightFences[i]);
        if (result != VK_SUCCESS)
            throw std::runtime_error("Error creating inFlight fence");
    }
}

bool Velox::ForwardSDLEventToRenderer(SDL_Event* event)
{
    switch (event->type)
    {
        case SDL_EVENT_WINDOW_RESIZED:               g_frameBufferResized = true; break;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:    g_frameBufferResized = true; break;
        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED: g_frameBufferResized = true; break;
        default: break;
    }

    return true;
}

void Velox::StartFrame()
{
    vkDeviceWaitIdle(g_device);

    g_vertexCount = 0;
    g_indexCount = 0;
}

void Velox::DrawFrame()
{
    // Check if window is currently minimised.
    if (SDL_GetWindowFlags(g_window) & SDL_WINDOW_MINIMIZED)
        return;

    vkWaitForFences(g_device, 1, &g_inFlightFences[g_currentFrame], VK_TRUE, UINT64_MAX); // Want for frame to finish.

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(g_device, g_swapchain, UINT64_MAX, 
            g_imageAvailableSemaphores[g_currentFrame], VK_NULL_HANDLE, &imageIndex);

    // printf("AQUIREIMAGE(): %s\n", string_VkResult(result));

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        Velox::ReCreateSwapchain();
        return;
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to aquire next image");
    }

    Velox::UpdateUniformBuffer(g_currentFrame);
    
    vkResetFences(g_device, 1, &g_inFlightFences[g_currentFrame]); // Resent fence for next wait.

    vkResetCommandBuffer(g_commandBuffers[g_currentFrame], 0);

    Velox::RecordCommandBuffer(g_commandBuffers[g_currentFrame], imageIndex);

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { g_imageAvailableSemaphores[g_currentFrame] };
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &g_commandBuffers[g_currentFrame];

    VkSemaphore signalSemaphores[] = { g_renderFinishedSemaphores[g_currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    result = vkQueueSubmit(g_graphicsQueue, 1, &submitInfo, g_inFlightFences[g_currentFrame]);
    if (result != VK_SUCCESS)
    {
        printf("ERROR: vkQueueSubmit(): %s\n", string_VkResult(result));
        throw std::runtime_error("Error submitting to graphics queue");
    }

    VkPresentInfoKHR presentInfo {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { g_swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    result = vkQueuePresentKHR(g_presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || g_frameBufferResized)
    {
        g_frameBufferResized = false;
        Velox::ReCreateSwapchain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swapchain image");
    }

    g_currentFrame = (g_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Velox::EndFrame()
{
    Velox::DoCopyPass();
    Velox::DrawFrame();
    // GM: For now we just insert engine stuff here.
    // Mosly just drawing engine UI elements.
    // Velox::DoFrameEndUpdates();

    // GM: Finalises and generates ImGui draw data.
    // ImGui::Render();
}

void Velox::DoRenderPass()
{
}

void Velox::DoCopyPass()
{
    Velox::CopyVerticesToGPU();
    Velox::CopyIndicesToGPU();
}

void Velox::CopyVerticesToGPU()
{
    VkDeviceSize bufferSize = sizeof(g_vertices);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    Velox::CreateBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void* data;
    vkMapMemory(g_device, stagingBufferMemory, 0, bufferSize, 0, &data);

    memcpy(data, g_vertices, (size_t)bufferSize);

    vkUnmapMemory(g_device, stagingBufferMemory);

    Velox::CopyBuffer(stagingBuffer, g_vertexBuffer, bufferSize);

    vkDestroyBuffer(g_device, stagingBuffer, nullptr);
    vkFreeMemory(g_device, stagingBufferMemory, nullptr);
}

void Velox::CopyIndicesToGPU()
{
    VkDeviceSize bufferSize = sizeof(g_indices);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    Velox::CreateBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void* data;
    vkMapMemory(g_device, stagingBufferMemory, 0, bufferSize, 0, &data);

    memcpy(data, g_indices, (size_t) bufferSize);

    vkUnmapMemory(g_device, stagingBufferMemory);

    Velox::CopyBuffer(stagingBuffer, g_indexBuffer, bufferSize);

    vkDestroyBuffer(g_device, stagingBuffer, nullptr);
    vkFreeMemory(g_device, stagingBufferMemory, nullptr);
}

void Velox::DeInitRenderer()
{
    vkDeviceWaitIdle(g_device);

    Velox::CleanupSwapchain();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(g_device, g_uniformBuffers[i], nullptr);
        vkFreeMemory(g_device, g_uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(g_device, g_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(g_device, g_descriptorSetLayout, nullptr);

    vkDestroyBuffer(g_device, g_indexBuffer, nullptr);
    vkFreeMemory(g_device, g_indexBufferMemory, nullptr);

    vkDestroyBuffer(g_device, g_vertexBuffer, nullptr);
    vkFreeMemory(g_device, g_vertexBufferMemory, nullptr);


    vkDestroyPipeline(g_device, g_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(g_device, g_pipelineLayout, nullptr);

    vkDestroyRenderPass(g_device, g_renderPass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(g_device, g_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(g_device, g_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(g_device, g_inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(g_device, g_commandPool, nullptr);

    vkDestroyDevice(g_device, nullptr);

    SDL_Vulkan_DestroySurface(g_instance, g_surface, nullptr);
    vkDestroyInstance(g_instance, nullptr);

    SDL_DestroyWindow(g_window);
}

// filepath relative to the exe (in build/bin).
void Velox::LoadShader(VkShaderModule* shaderModule, const char* filepath, Velox::Arena* allocator)
{
    size_t bufferSize = sizeof(char) * 1024;

    char* absoluteFilepath = allocator->Alloc<char>(bufferSize);
    SDL_strlen(absoluteFilepath);

    SDL_strlcpy(absoluteFilepath, SDL_GetBasePath(), bufferSize);
    SDL_strlcat(absoluteFilepath, "shaders\\", bufferSize);
    SDL_strlcat(absoluteFilepath, filepath, bufferSize);

    std::ifstream file(absoluteFilepath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw std::runtime_error("failed to open file");

    size_t fileSize = (size_t)file.tellg();
    char* shaderCode = allocator->Alloc<char>(fileSize);;

    file.seekg(0);
    file.read(shaderCode, fileSize);

    file.close();

    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = fileSize;
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode);

    VkResult result = vkCreateShaderModule(g_device, &shaderModuleCreateInfo, nullptr, shaderModule);
    if (result != VK_SUCCESS)
    {
        printf("ERROR: vkCreateShaderModule(): %s\n", string_VkResult(result));
        throw std::runtime_error("Failed to create Vulkan ShaderModule");
    }
}

void Velox::LoadImage(const char* filepath)
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
        // return nullptr;
    }

    if (surface->format != SDL_PIXELFORMAT_ABGR8888)
    {
        printf("Error: Loaded image has wrong pixel format. Expected \"SDL_PIXELFORMAT_ABGR888\", \
            found: \"%s\"\n", SDL_GetPixelFormatName(surface->format));

        // This is safe for now.
        // TODO: Atempt to convert maybe?

        // return nullptr;
    }

    // return surface;
}

void Velox::CreateTexture()
{
}

uint32_t Velox::AddVertex(Velox::Vertex vertex)
{
    int index = g_vertexCount;

    g_vertices[g_vertexCount] = vertex;
    g_vertexCount++;

    return index;
}

void Velox::AddIndex(uint32_t index)
{
    g_indices[g_indexCount] = index;
    g_indexCount++;
}

