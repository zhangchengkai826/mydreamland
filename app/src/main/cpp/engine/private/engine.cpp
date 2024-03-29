//
// Created by andys on 6/1/2019.
//

#include "engine.h"

#include <mdlcg.h>

void Engine::init() {
    pipelines = new std::map<std::string, VkPipeline>();
    object3ds = new std::map<std::string, Object3D>();
    object2ds = new std::map<std::string, Object2D>();

#ifdef DEBUG
    validationLayerProperties = new std::vector<VkLayerProperties>();
    validationLayerNames = new std::vector<const char *>();
    updateAvailableValidationLayerNames();
#endif

    createVKInstance();

#ifdef DEBUG
    createVKDebugReportCallback();
#endif

    createVKAndroidSurface();
    selectPhysicalDevice();
    updatePhysicalDeviceSurfaceCapabilities();
    selectPhysicalDeviceSurfaceFormat();
    selectPhysicalDeviceSurfacePresentMode();
    updatePhysicalDeviceFeatures();
    updatePhysicalDeviceGraphicsQueueFamilyIndex();
    createLogicalDevice();
    vkGetDeviceQueue(vkDevice, physicalDeviceGraphicsQueueFamilyIndex, 0, &graphicsQueue);

    createSwapChain();
    createDepthStencilResources();
    createRenderPass();
    createFrameBuffers();

    createCmdPool();
    allocFrameCmdBuffers();

    createSamplers();

    createDescriptorSetLayouts();
    createDescriptorPools();

    createPipelineLayout();
    create3DPipeline();
    create2DPipeline();

    createFrameSyncObjs();
    currentFrame = 0;

    loadResources();
    prefillStaticSets();
}

void Engine::destroy() {
    vkDeviceWaitIdle(vkDevice);

    for(auto it = object2ds->begin(); it != object2ds->end(); it++) {
        it->second.destroy();
    }
    for(auto it = object3ds->begin(); it != object3ds->end(); it++) {
        it->second.destroy();
    }
    resourceMgr.destroy(this);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyFence(vkDevice, inFlightFrameFences[i], nullptr);
        vkDestroySemaphore(vkDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(vkDevice, imageAvailableSemaphores[i], nullptr);
    }

    for(auto it = pipelines->begin(); it != pipelines->end(); it++) {
        vkDestroyPipeline(vkDevice, it->second, nullptr);
    }
    vkDestroyPipelineLayout(vkDevice, pipelineLayout, nullptr);

    vkDestroyBuffer(vkDevice, uniformBuffer, nullptr);
    vkFreeMemory(vkDevice, uniformBuffersMemory, nullptr);
    vkDestroyDescriptorPool(vkDevice, staticDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(vkDevice, staticSetLayout, nullptr);
    vkDestroySampler(vkDevice, sampler, nullptr);

    vkDestroyCommandPool(vkDevice, commandPool, nullptr);

    for(auto framebuffer: swapChainFrameBuffers) {
        vkDestroyFramebuffer(vkDevice, framebuffer, nullptr);
    }
    vkDestroyRenderPass(vkDevice, renderPass, nullptr);

    for(int i = 0; i < NUM_IMAGES_IN_SWAPCHAIN; i++) {
        vkDestroyImageView(vkDevice, depthStencilImageViews[i], nullptr);
        vkDestroyImage(vkDevice, depthStencilImages[i], nullptr);
        vkFreeMemory(vkDevice, depthStencilImageMemorys[i], nullptr);
    }

    for(auto imageView: swapChainImageViews) {
        vkDestroyImageView(vkDevice, imageView, nullptr);
    }
    vkDestroySwapchainKHR(vkDevice, vkSwapchain, nullptr);

    vkDestroyDevice(vkDevice, nullptr);
    vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);

#ifdef DEBUG
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
    vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)
            vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
    vkDestroyDebugReportCallbackEXT(vkInstance, vkDebugReportCallbackExt, nullptr);
#endif

    vkDestroyInstance(vkInstance, nullptr);

#ifdef DEBUG
    delete validationLayerNames;
    delete validationLayerProperties;
#endif

    delete object2ds;
    delete object3ds;
    delete pipelines;
}

void Engine::drawFrame() {
    vkWaitForFences(vkDevice, 1, &inFlightFrameFences[currentFrame], VK_TRUE,
                    std::numeric_limits<uint64_t>::max());
    vkResetFences(vkDevice, 1, &inFlightFrameFences[currentFrame]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(vkDevice, vkSwapchain,
                          std::numeric_limits<uint64_t>::max(),
                          imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    recordFrameCmdBuffers(imageIndex);

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &frameCommandBuffers[currentFrame],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores,
    };
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFrameFences[currentFrame]);

    VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores,
            .swapchainCount = 1,
            .pSwapchains = &vkSwapchain,
            .pImageIndices = &imageIndex,
            .pResults = nullptr,
    };
    vkQueuePresentKHR(graphicsQueue, &presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

