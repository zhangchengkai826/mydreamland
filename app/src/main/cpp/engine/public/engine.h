//
// Created by andys on 5/31/2019.
//

#ifndef MYDREAMLAND_ENGINE_H
#define MYDREAMLAND_ENGINE_H

#include "../../../../../../../../AppData/Local/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/c++/v1/vector"

#include "../../../../../../../../AppData/Local/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/android/log.h"
#include "../../../../../../../../AppData/Local/Android/Sdk/ndk-bundle/sources/android/native_app_glue/android_native_app_glue.h"

#include "../../../../../../../../AppData/Local/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/vulkan/vulkan.h"
#include "../../../../../../../../AppData/Local/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/vulkan/vulkan_android.h"

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
    VkDebugReportCallbackEXT vkDebugReportCallbackExt = VK_NULL_HANDLE;
    VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
    VkPhysicalDevice vkGpu = VK_NULL_HANDLE;
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

    /* engine_shader_helper.cpp */
    std::vector<char> readFile(const char *fileName);
    VkShaderModule createShaderModule(const std::vector<char> &code);
};

extern Engine gEngine;

#endif //MYDREAMLAND_ENGINE_H
