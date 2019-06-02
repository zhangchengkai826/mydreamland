//
// Created by andys on 5/31/2019.
//

#ifndef MYDREAMLAND_ENGINE_H
#define MYDREAMLAND_ENGINE_H

#include <vector>

#include <android/log.h>
#include <android_native_app_glue.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

class Engine {
public:
    struct SavedState {
        int32_t x = 0;
        int32_t y = 0;
    };

    struct android_app *app = nullptr;
    struct SavedState state;
    bool animating = false;

    static void cmdHandler(struct android_app *app, int32_t cmd);
    static int32_t inputHandler(struct android_app *app, AInputEvent *event);

    void drawFrame();

private:
    constexpr static int MAX_FRAMES_IN_FLIGHT = 2;
    constexpr static bool DEBUG_ON = true;

    VkInstance vkInstance = VK_NULL_HANDLE;
    std::vector<const char *> validationLayerNames;
    VkDebugReportCallbackEXT vkDebugReportCallbackExt = VK_NULL_HANDLE;
    VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
    VkDevice vkDevice = VK_NULL_HANDLE;
    VkQueue vkQueue = VK_NULL_HANDLE;
    VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;
    VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> swapChainFrameBuffers;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    int currentFrame = 0;

    /* engine.cpp */
    void cmdHandlerInternal(struct android_app *app, int32_t cmd);
    int32_t inputHandlerInternal(struct android_app *app, AInputEvent *event);
    int initDisplay();
    void termDisplay();

    /* engine_helper.cpp */
    void updateAvailableValidationLayerNames();
    void createVKInstance();
    void createVKAndroidSurface();
    void selectPhysicalDevice();

    /* engine_shader_helper.cpp */
    std::vector<char> readFile(const char *fileName);
    VkShaderModule createShaderModule(const std::vector<char> &code);

    /* engine_debug_output.cpp */
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(
            VkDebugReportFlagsEXT msgFlags,
            VkDebugReportObjectTypeEXT objType,
            uint64_t srcObject, size_t location,
            int32_t msgCode, const char *pLayerPrefix,
            const char *pMsg, void *pUserData);
    void printAvailableInstanceExtensions();
    void createVKDebugReportCallback();
};

extern Engine gEngine;

#endif //MYDREAMLAND_ENGINE_H
