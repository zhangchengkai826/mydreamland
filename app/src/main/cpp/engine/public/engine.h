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
#include <fstream>
#include <string>

#include <android/native_activity.h>
#include <android/log.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stb_image.h>

#define DEBUG

class Engine;

std::ifstream &operator>>(std::ifstream &f, glm::vec3 &v);
std::ifstream &operator>>(std::ifstream &f, glm::vec2 &v);

class Vertex {
public:
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};
std::ifstream &operator>>(std::ifstream &f, Vertex &v);

class Geometry {
public:
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    uint32_t nVertices;
    uint32_t nIndices;

    void initFromFile(const Engine *engine, const char *filename);
    void destroy(const Engine *engine);
};

class Texture {
public:
    VkImage image;
    VkDeviceMemory imageMemory;
    VkImageView imageView;
    VkSampler sampler;

    void initFromFile(const Engine *engine, const char *fileName);
    void destroy(const Engine *engine);
};

struct UniformBuffer {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class Engine {
    friend class Geometry;
    friend class Texture;
public:
    pthread_mutex_t mutex;

    ANativeActivity *activity;
    ANativeWindow *window;

    int fpsFrameCounter;
    bool bAnimating;

    void init();
    void destroy();

    void drawFrame();

private:
    constexpr static int NUM_IMAGES_IN_SWAPCHAIN = 3;
    constexpr static int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef DEBUG
    std::vector<VkLayerProperties> validationLayerProperties;
    std::vector<const char *> validationLayerNames;
#endif

    VkInstance vkInstance;
    VkDebugReportCallbackEXT vkDebugReportCallbackExt;

    VkSurfaceKHR vkSurface;
    VkPhysicalDevice vkPhysicalDevice;
    VkSurfaceCapabilitiesKHR physicalDeviceSurfaceCapabilities;
    VkSurfaceFormatKHR physicalDeviceSurfaceFormat;
    VkPresentModeKHR physicalDeviceSurfacePresentMode;
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    uint32_t physicalDeviceGraphicsQueueFamilyIndex;
    VkDevice vkDevice;
    VkQueue graphicsQueue;

    VkSwapchainKHR vkSwapchain;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    VkImage depthStencilImage;
    VkDeviceMemory depthStencilImageMemory;
    VkImageView depthStencilImageView;
    VkRenderPass renderPass;
    std::vector<VkFramebuffer> swapChainFrameBuffers;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkFence> inFlightFences;
    int currentFrame;

    Geometry geometry;
    Texture texture;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    VkPipelineLayout graphicsPipelineLayout;
    VkPipeline graphicsPipeline;

    /* engine_helper.cpp */
    void createVKInstance();
    void createVKAndroidSurface();

    void selectPhysicalDevice();
    void updatePhysicalDeviceSurfaceCapabilities();
    void selectPhysicalDeviceSurfaceFormat();
    void selectPhysicalDeviceSurfacePresentMode();
    void updatePhysicalDeviceFeatures();
    void updatePhysicalDeviceGraphicsQueueFamilyIndex();
    void createLogicalDevice();

    void createSwapChain();
    void createDepthStencilResources();
    void createRenderPass();
    void createFrameBuffers();

    void createCmdPool();
    void allocCmdBuffers();

    void createSyncObjs();

    void loadResources();
    void destroyResources();

    void createUniformBuffers();

    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSets();
    void createGraphicsPipelineLayout();
    void createGraphicsPipeline();

    void recordCmdBuffers();

    VkCommandBuffer beginSingleTimeCommands() const;
    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

    /* engine_physics.cpp */
    void updateUniformBuffer(uint32_t imageIndex);

    /* engine_texture.cpp */

    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags propertyFlags, VkImage &image,
                     VkDeviceMemory &imageMemory) const;
    void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image,
                               VkImageAspectFlags aspectFlags, VkImageLayout oldLayout,
                               VkImageLayout newLayout, uint32_t mipLevels) const;
    void copyBufferToImage(VkCommandBuffer commandBuffer,
            VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                                uint32_t mipLevels) const;

    /* engine_shader_helper.cpp */
    std::vector<char> readFile(const char *fileName);
    VkShaderModule createShaderModule(const std::vector<char> &code);

    /* engine_buffer_helper.cpp */
    uint32_t findOptimalMemoryTypeIndexSupportSpecifiedPropertyFlags(uint32_t targetMemoryypeBits,
            VkMemoryPropertyFlags targetMemoryPropertyFlags) const;
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
            VkMemoryPropertyFlags propertyFlags, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
            const;
    void copyBuffer(VkBuffer srcBuffer, VkBuffer destBuffer, VkDeviceSize size) const;

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
};

#endif //MYDREAMLAND_ENGINE_H
