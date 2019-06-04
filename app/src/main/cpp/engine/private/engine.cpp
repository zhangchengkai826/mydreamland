//
// Created by andys on 6/1/2019.
//

#include "engine.h"

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
};
const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

void Engine::init() {
    if(DEBUG_ON) {
        updateAvailableValidationLayerNames();
    }
    createVKInstance();
    if(DEBUG_ON && validationLayerNames.size() > 0) {
        createVKDebugReportCallback();
    }
    selectPhysicalDevice();
    updatePhysicalDeviceFeatures();
    updatePhysicalDeviceGraphicsQueueFamilyIndex();
    createLogicalDevice();
    vkGetDeviceQueue(vkDevice, physicalDeviceGraphicsQueueFamilyIndex, 0, &graphicsQueue);

    createRenderPass();

    createDescriptorSetLayout();
    createGraphicsPipelineLayout();

    createCmdPool();
    createDescriptorPool();

    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorSets();
    createTextureImage();
    createTextureImageView();
    createTextureSampler();

    createSyncObjs();
}

void Engine::destroy() {
    vkDeviceWaitIdle(vkDevice);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyFence(vkDevice, inFlightFences[i], nullptr);
        vkDestroySemaphore(vkDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(vkDevice, imageAvailableSemaphores[i], nullptr);
    }

    vkDestroySampler(vkDevice, textureSampler, nullptr);
    vkDestroyImageView(vkDevice, textureImageView, nullptr);
    vkDestroyImage(vkDevice, textureImage, nullptr);
    vkFreeMemory(vkDevice, textureImageMemory, nullptr);
    for(size_t i = 0; i < NUM_IMAGES_IN_SWAPCHAIN; i++) {
        vkDestroyBuffer(vkDevice, uniformBuffers[i], nullptr);
        vkFreeMemory(vkDevice, uniformBuffersMemory[i], nullptr);
    }
    vkDestroyBuffer(vkDevice, indexBuffer, nullptr);
    vkFreeMemory(vkDevice, indexBufferMemory, nullptr);
    vkDestroyBuffer(vkDevice, vertexBuffer, nullptr);
    vkFreeMemory(vkDevice, vertexBufferMemory, nullptr);

    vkDestroyDescriptorPool(vkDevice, descriptorPool, nullptr);
    vkDestroyCommandPool(vkDevice, commandPool, nullptr);

    vkDestroyPipelineLayout(vkDevice, graphicsPipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(vkDevice, descriptorSetLayout, nullptr);

    vkDestroyRenderPass(vkDevice, renderPass, nullptr);

    vkDestroyDevice(vkDevice, nullptr);
    if(DEBUG_ON && validationLayerNames.size() > 0) {
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
        vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)
                vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
        vkDestroyDebugReportCallbackEXT(vkInstance, vkDebugReportCallbackExt, nullptr);
    }
    vkDestroyInstance(vkInstance, nullptr);
}

void Engine::initDisplay() {
    createVKAndroidSurface();
    updatePhysicalDeviceSurfaceCapabilities();
    checkPhysicalDeviceSurfaceFormatSupport();

    createSwapChain();

    createGraphicsPipeline();

    createFrameBuffers();

    allocCmdBuffers();
    recordCmdBuffers();
}

void Engine::destroyDisplay() {
    vkDeviceWaitIdle(vkDevice);

    for(auto framebuffer: swapChainFrameBuffers) {
        vkDestroyFramebuffer(vkDevice, framebuffer, nullptr);
    }

    vkDestroyPipeline(vkDevice, graphicsPipeline, nullptr);

    for(auto imageView: swapChainImageViews) {
        vkDestroyImageView(vkDevice, imageView, nullptr);
    }
    vkDestroySwapchainKHR(vkDevice, vkSwapchain, nullptr);

    vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
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
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffers[imageIndex],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores,
    };
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);

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

