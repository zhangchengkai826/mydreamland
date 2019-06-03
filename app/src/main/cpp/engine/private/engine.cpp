//
// Created by andys on 6/1/2019.
//

#include "engine.h"

const std::vector<Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
};

Engine gEngine;

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
}

void Engine::destroy() {
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

int Engine::initDisplay() {
    createVKAndroidSurface();
    updatePhysicalDeviceSurfaceCapabilities();
    checkPhysicalDeviceSurfaceFormatSupport();
    createSwapChain();

    createGraphicsPipeline();
    createFrameBuffers();

    createCmdPool();
    createVertexBuffer();
    allocCmdBuffers();
    recordCmdBuffers();

    createSyncObjs();

    return 0;
}

void Engine::drawFrame() {
    vkWaitForFences(vkDevice, 1, &inFlightFences[currentFrame], VK_TRUE,
                    std::numeric_limits<uint64_t>::max());
    vkResetFences(vkDevice, 1, &inFlightFences[currentFrame]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(vkDevice, vkSwapchain,
                          std::numeric_limits<uint64_t>::max(),
                          imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

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

void Engine::termDisplay() {
    vkDeviceWaitIdle(vkDevice);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyFence(vkDevice, inFlightFences[i], nullptr);
        vkDestroySemaphore(vkDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(vkDevice, imageAvailableSemaphores[i], nullptr);
    }

    vkDestroyBuffer(vkDevice, vertexBuffer, nullptr);
    vkFreeMemory(vkDevice, vertexBufferMemory, nullptr);
    vkDestroyCommandPool(vkDevice, commandPool, nullptr);

    for(auto framebuffer: swapChainFrameBuffers) {
        vkDestroyFramebuffer(vkDevice, framebuffer, nullptr);
    }

    vkDestroyPipeline(vkDevice, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(vkDevice, graphicsPipelineLayout, nullptr);

    for(auto imageView: swapChainImageViews) {
        vkDestroyImageView(vkDevice, imageView, nullptr);
    }
    vkDestroySwapchainKHR(vkDevice, vkSwapchain, nullptr);
    vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
}

void Engine::cmdHandler(struct android_app *app, int32_t cmd) {
    gEngine.cmdHandlerInternal(app, cmd);
}

void Engine::cmdHandlerInternal(struct android_app *app, int32_t cmd) {
    auto engine = reinterpret_cast<Engine *>(app->userData);
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            app->savedState = malloc(sizeof(struct SavedState));
            *((struct SavedState*)app->savedState) = engine->state;
            app->savedStateSize = sizeof(struct SavedState);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            initDisplay();
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            termDisplay();
            break;
        case APP_CMD_GAINED_FOCUS:
            break;
        case APP_CMD_LOST_FOCUS:
            engine->animating = false;
            break;
        default:
            break;
    }
}

int32_t Engine::inputHandler(struct android_app *app, AInputEvent *event) {
    return gEngine.inputHandlerInternal(app, event);
}

int32_t Engine::inputHandlerInternal(struct android_app *app, AInputEvent *event) {
    auto engine = reinterpret_cast<Engine *>(app->userData);
    if(AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = true;
        engine->state.x = (int32_t)AMotionEvent_getX(event, 0);
        engine->state.y = (int32_t)AMotionEvent_getY(event, 0);
        __android_log_print(ANDROID_LOG_INFO, "main", "x: %d, y: %d",
                engine->state.x, engine->state.y);
        return 1;
    }
    return 0;
}

