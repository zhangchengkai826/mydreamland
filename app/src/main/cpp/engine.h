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

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "main", __VA_ARGS__)

struct saved_state {
    int32_t x = 0;
    int32_t y = 0;
};

struct engine {
    struct android_app *app = nullptr;

    int animating = 0;
    struct saved_state state;

    VkInstance vkInstance = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT vkDebugReportCallbackExt = VK_NULL_HANDLE;
    VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
    VkPhysicalDevice vkGpu = VK_NULL_HANDLE;
    VkDevice vkDevice = VK_NULL_HANDLE;
    VkQueue vkQueue = VK_NULL_HANDLE;
    VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;
    VkFormat swapChainImageFormat;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFrameBuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
};

#endif //MYDREAMLAND_ENGINE_H
