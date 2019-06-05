//
// Created by andys on 5/31/2019.
//

#ifndef MYDREAMLAND_ENGINE_H
#define MYDREAMLAND_ENGINE_H

#include <pthread.h>

#include <vector>
#include <array>
#include <queue>
#include <chrono>
#include <exception>

#include <android/native_activity.h>
#include <android/log.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stb_image.h>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };
        return bindingDescription;
    }
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{{
            {.binding = 0,
             .location = 0,
             .format = VK_FORMAT_R32G32B32_SFLOAT,
             .offset = offsetof(Vertex, pos)},
            {.binding = 0,
             .location = 1,
             .format = VK_FORMAT_R32G32B32_SFLOAT,
             .offset = offsetof(Vertex, color)},
            {.binding = 0,
             .location = 2,
             .format = VK_FORMAT_R32G32_SFLOAT,
             .offset = offsetof(Vertex, texCoord)},
        }};
        return attributeDescriptions;
    }
};

struct UniformBuffer {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

extern const std::vector<Vertex> vertices;
extern const std::vector<uint16_t> indices;

class Engine {
public:
    ANativeActivity *activity = nullptr;
    ANativeWindow *window = nullptr;

    bool bDisplayInited = false;
    bool bAnimating = true;

    int fpsFrameCounter = 0;

    void init();
    void destroy();
    void initDisplay();
    void destroyDisplay();

    void drawFrame();

private:
    constexpr static int NUM_IMAGES_IN_SWAPCHAIN = 3;
    constexpr static int MAX_FRAMES_IN_FLIGHT = 2;
    constexpr static bool DEBUG_ON = true;

    VkInstance vkInstance = VK_NULL_HANDLE;
    std::vector<VkLayerProperties> validationLayerProperties;
    std::vector<const char *> validationLayerNames;
    VkDebugReportCallbackEXT vkDebugReportCallbackExt = VK_NULL_HANDLE;

    VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    VkSurfaceCapabilitiesKHR physicalDeviceSurfaceCapabilities;
    uint32_t physicalDeviceGraphicsQueueFamilyIndex = 0;
    VkSurfaceFormatKHR physicalDeviceSurfaceFormat = {VK_FORMAT_R8G8B8A8_UNORM,
                                   VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    VkPresentModeKHR physicalDeviceSurfacePresentMode = VK_PRESENT_MODE_FIFO_KHR;

    VkDevice vkDevice = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;

    VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkRenderPass renderPass = VK_NULL_HANDLE;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> swapChainFrameBuffers;

    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> descriptorSets;

    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    VkImage textureImage = VK_NULL_HANDLE;
    VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
    VkImageView textureImageView = VK_NULL_HANDLE;
    VkSampler textureSampler = VK_NULL_HANDLE;

    VkImage depthStencilImage;
    VkDeviceMemory depthStencilImageMemory;
    VkImageView depthStencilImageView;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    int currentFrame = 0;

    /* engine_helper.cpp */
    void createVKInstance();
    void createVKAndroidSurface();

    void selectPhysicalDevice();
    void updatePhysicalDeviceFeatures();
    void updatePhysicalDeviceSurfaceCapabilities();
    void updatePhysicalDeviceGraphicsQueueFamilyIndex();

    void createLogicalDevice();
    void createSwapChain();

    void createRenderPass();
    void createDescriptorSetLayout();
    void createGraphicsPipelineLayout();
    void createGraphicsPipeline();
    void createFrameBuffers();

    void createCmdPool();
    void allocCmdBuffers();
    void recordCmdBuffers();

    void createDescriptorPool();
    void createDescriptorSets();

    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();

    void createSyncObjs();

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    /* engine_physics.cpp */
    void updateUniformBuffer(uint32_t imageIndex);

    /* engine_image.cpp */
    void createTextureImage();
    void createTextureImageView();
    void createTextureSampler();
    void createDepthStencilResources();

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage, VkMemoryPropertyFlags propertyFlags, VkImage &image,
            VkDeviceMemory &imageMemory);
    void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image,
            VkImageAspectFlags aspectFlags, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkCommandBuffer commandBuffer,
            VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    /* engine_shader_helper.cpp */
    std::vector<char> readFile(const char *fileName);
    VkShaderModule createShaderModule(const std::vector<char> &code);

    /* engine_buffer_helper.cpp */
    uint32_t findOptimalMemoryTypeIndexSupportSpecifiedPropertyFlags(uint32_t targetMemoryypeBits,
            VkMemoryPropertyFlags targetMemoryPropertyFlags);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
            VkMemoryPropertyFlags propertyFlags, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer destBuffer, VkDeviceSize size);

    /* engine_debug_output.cpp */
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(
            VkDebugReportFlagsEXT msgFlags,
            VkDebugReportObjectTypeEXT objType,
            uint64_t srcObject, size_t location,
            int32_t msgCode, const char *pLayerPrefix,
            const char *pMsg, void *pUserData);
    void logAvailableInstanceExtensions();
    void updateAvailableValidationLayerNames();
    void createVKDebugReportCallback();
    void logPhysicalDeviceProperties();
    void logPhysicalDeviceAvailableExtensions();
    void checkPhysicalDeviceGraphicsQueueSurfaceSupport();
    void checkPhysicalDeviceSurfaceFormatSupport();
    void checkPhysicalDeviceSurfacePresentModeSupport();
};

#endif //MYDREAMLAND_ENGINE_H
