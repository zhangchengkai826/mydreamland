//
// Created by andys on 6/1/2019.
//

#include "engine.h"

void Engine::init() {

#ifdef DEBUG
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
    allocCmdBuffers();

    createSyncObjs();
    currentFrame = 0;

    loadResources();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();

    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSets();
    createGraphicsPipelineLayout();
    createGraphicsPipeline();

    recordCmdBuffers();
}

void Engine::destroy() {
    vkDeviceWaitIdle(vkDevice);

    vkDestroyPipeline(vkDevice, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(vkDevice, graphicsPipelineLayout, nullptr);
    vkDestroyDescriptorPool(vkDevice, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(vkDevice, descriptorSetLayout, nullptr);

    for(size_t i = 0; i < NUM_IMAGES_IN_SWAPCHAIN; i++) {
        vkDestroyBuffer(vkDevice, uniformBuffers[i], nullptr);
        vkFreeMemory(vkDevice, uniformBuffersMemory[i], nullptr);
    }
    vkDestroyBuffer(vkDevice, indexBuffer, nullptr);
    vkFreeMemory(vkDevice, indexBufferMemory, nullptr);
    vkDestroyBuffer(vkDevice, vertexBuffer, nullptr);
    vkFreeMemory(vkDevice, vertexBufferMemory, nullptr);
    texture.destroy(this);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyFence(vkDevice, inFlightFences[i], nullptr);
        vkDestroySemaphore(vkDevice, imageAvailableSemaphores[i], nullptr);
    }

    vkDestroyCommandPool(vkDevice, commandPool, nullptr);

    for(auto framebuffer: swapChainFrameBuffers) {
        vkDestroyFramebuffer(vkDevice, framebuffer, nullptr);
    }
    vkDestroyRenderPass(vkDevice, renderPass, nullptr);
    vkDestroyImageView(vkDevice, depthStencilImageView, nullptr);
    vkDestroyImage(vkDevice, depthStencilImage, nullptr);
    vkFreeMemory(vkDevice, depthStencilImageMemory, nullptr);
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
}

void Engine::drawFrame() {
    vkWaitForFences(vkDevice, 1, &inFlightFences[currentFrame], VK_TRUE,
                    std::numeric_limits<uint64_t>::max());
    vkResetFences(vkDevice, 1, &inFlightFences[currentFrame]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(vkDevice, vkSwapchain,
                          std::numeric_limits<uint64_t>::max(),
                          imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    updateUniformBuffer(imageIndex);

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
    VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffers[imageIndex],
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr,
    };
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);

    VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .swapchainCount = 1,
            .pSwapchains = &vkSwapchain,
            .pImageIndices = &imageIndex,
            .pResults = nullptr,
    };
    vkQueuePresentKHR(graphicsQueue, &presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

