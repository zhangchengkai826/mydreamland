//
// Created by andys on 5/31/2019.
//

#ifndef MYDREAMLAND_ENGINE_H
#define MYDREAMLAND_ENGINE_H

#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>

#include <vector>
#include <array>
#include <queue>
#include <chrono>
#include <exception>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <atomic>

#include <android/native_activity.h>
#include <android/log.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/spline.hpp>
#include <glm/gtx/string_cast.hpp>

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

    void initFromFile(Engine *engine, VkCommandBuffer commandBuffer,
                      const char *filename);
    void destroy(Engine *engine);
};

class Texture {
public:
    VkImage image;
    VkDeviceMemory imageMemory;
    VkImageView imageView;

    void initFromFile(Engine *engine, VkCommandBuffer commandBuffer, const char *fileName);
    void destroy(Engine *engine);

private:
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
};

class AnimController {
public:
    float t; /* current t-axis value */
    float tMax; /* t-axis value range: [0, tMax) */

    /* 9 anim curves. (t-axis, v-axis)
     * auto-replay.
     * each curve use catmull-rom spline, so at-lease has 4 control points.
     */
    std::vector<glm::vec2> posX;
    std::vector<glm::vec2> posY;
    std::vector<glm::vec2> posZ;

    std::vector<glm::vec2> rotX;
    std::vector<glm::vec2> rotY;
    std::vector<glm::vec2> rotZ;

    std::vector<glm::vec2> scaleX;
    std::vector<glm::vec2> scaleY;
    std::vector<glm::vec2> scaleZ;

    glm::mat4 advance(float dt);

    float interpolate(std::vector<glm::vec2> &curve);
};

class Object3D {
public:
    void init(Geometry *geo, Texture *tex);
    void destroy();

    void setPostion(float x, float y, float z);

    void refreshModelMat();

    Geometry *geo;
    Texture *tex;

    glm::mat4 modelMat;

    AnimController animController;
};

class Engine {
    friend class Geometry;
    friend class Texture;
public:
    pthread_mutex_t mutex;

    /* render & physics threads both access them */
    std::atomic_int *fpsFrameCounter;
    std::map<std::string, Object3D> *object3ds; /* assume now that it's fixed-length after init() in main thread */

    /* physics thread access them */

    /* render thread access them */
    ANativeActivity *activity;
    ANativeWindow *window;

    bool bAnimating;

    void init();
    void destroy();

    void drawFrame();

private:
    constexpr static int NUM_IMAGES_IN_SWAPCHAIN = 3;
    constexpr static int MAX_FRAMES_IN_FLIGHT = 2;
    constexpr static int MAX_TEXTURES_PER_FRAME = 8;

#ifdef DEBUG
    std::vector<VkLayerProperties> *validationLayerProperties;
    std::vector<const char *> *validationLayerNames;
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
    std::vector<VkImage> *swapChainImages;
    std::vector<VkImageView> *swapChainImageViews;
    std::vector<VkImage> *depthStencilImages;
    std::vector<VkDeviceMemory> *depthStencilImageMemorys;
    std::vector<VkImageView> *depthStencilImageViews;
    VkRenderPass renderPass;
    std::vector<VkFramebuffer> *swapChainFrameBuffers;

    VkCommandPool commandPool;
    VkCommandBuffer frameCommandBuffers[MAX_FRAMES_IN_FLIGHT];

    std::vector<VkSemaphore> *imageAvailableSemaphores;
    std::vector<VkSemaphore> *renderFinishedSemaphores;
    std::vector<VkFence> *inFlightFrameFences;
    int currentFrame;

    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBuffersMemory;

    VkSampler sampler;

    VkDescriptorSetLayout staticSetLayout;
    VkDescriptorSetLayout resettableSetLayout;

    /* descriptor sets is allocated at init, and never gets reset.
     * the memory referenced by each descriptor may change each frame,
     * and after record vkCmdBindDescriptorSets, things referenced by descriptor set
     * (resources, device memory, etc) must not be changed until the command buffer finishes executing
     * on the GPU, so each frame should have its own descriptor set.
     */
    VkDescriptorPool staticDescriptorPool;
    VkDescriptorSet staticDescriptorSets[MAX_FRAMES_IN_FLIGHT];

    /* each frame will have its own resettable descriptor pool,
     * and at the start of each frame, the corresponding descriptor pool will get reset.
     */
    VkDescriptorPool resettableDescriptorPool[MAX_FRAMES_IN_FLIGHT];
    VkDescriptorSet resettableDescriptorSets[MAX_FRAMES_IN_FLIGHT];

    VkPipeline pipeline3D;
    VkPipelineLayout pipelineLayout3D;

    std::map<std::string, Geometry> *geometries;
    std::map<std::string, Texture> *textures;

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
    void allocFrameCmdBuffers();

    void createSamplers();

    void createDescriptorSetLayouts();
    void createDescriptorPools();
    void prefillStaticSets();

    void createPipelineLayout();
    void createPipeline();

    void createFrameSyncObjs();

    void loadResources();
    void destroyResources();

    void prepareResettableSets();
    void recordFrameCmdBuffers(int imageIndex);

    VkShaderModule createShaderModule(const char *fileName);

    VkCommandBuffer beginOneTimeSubmitCommands();
    void endOneTimeSubmitCommandsSyncWithFence(VkCommandBuffer commandBuffer);

    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags propertyFlags, VkImage &image,
                     VkDeviceMemory &imageMemory);
    void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image,
                               VkImageAspectFlags aspectFlags, VkImageLayout oldLayout,
                               VkImageLayout newLayout, uint32_t mipLevels,
                               VkPipelineStageFlags srcStageMask,
                               VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask,
                               VkAccessFlags dstAccessMask);
    void copyBufferToImage(VkCommandBuffer commandBuffer,
            VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                                uint32_t mipLevels);

    uint32_t findOptimalMemoryTypeIndexSupportSpecifiedPropertyFlags(uint32_t targetMemoryypeBits,
            VkMemoryPropertyFlags targetMemoryPropertyFlags);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
            VkMemoryPropertyFlags propertyFlags, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer destBuffer, VkDeviceSize size);

    /* debug */
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
