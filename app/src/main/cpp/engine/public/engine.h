//
// Created by andys on 5/31/2019.
//

#ifndef MYDREAMLAND_ENGINE_H
#define MYDREAMLAND_ENGINE_H

#include <pthread.h>

#include <vector>
#include <array>
#include <queue>

#include <android/native_activity.h>
#include <android/log.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

#include <glm/glm.hpp>

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };
        return bindingDescription;
    }
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{{
            {.binding = 0,
             .location = 0,
             .format = VK_FORMAT_R32G32_SFLOAT,
             .offset = offsetof(Vertex, pos)},
            {.binding = 0,
             .location = 1,
             .format = VK_FORMAT_R32G32B32_SFLOAT,
             .offset = offsetof(Vertex, color)},
        }};
        return attributeDescriptions;
    }
};

extern const std::vector<Vertex> vertices;
extern const std::vector<uint16_t> indices;

class Engine {
public:
    ANativeActivity *activity = nullptr;
    ANativeWindow *window = nullptr;
    bool bDisplayInited = false;
    int fpsFrameCounter = 0;

    bool animating = true;

    void init();
    void destroy();
    void initDisplay();
    void destroyDisplay();

    void drawFrame();

private:
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
    VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> swapChainFrameBuffers;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;
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
    void createGraphicsPipeline();
    void createFrameBuffers();

    void createCmdPool();
    void createVertexBuffer();
    void createIndexBuffer();
    void allocCmdBuffers();
    void recordCmdBuffers();

    void createSyncObjs();

    /* engine_shader_helper.cpp */
    std::vector<char> readFile(const char *fileName);
    VkShaderModule createShaderModule(const std::vector<char> &code);
    void createGraphicsPipelineLayout();

    /* engine_buffer_helper.cpp */
    uint32_t findOptimalMemoryTypeIndexSupportSpecifiedPropertyFlags(uint32_t targetMemoryTypeBits,
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
