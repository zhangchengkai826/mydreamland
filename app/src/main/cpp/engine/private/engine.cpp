//
// Created by andys on 6/1/2019.
//

#include "engine.h"

Engine gEngine;

int Engine::initDisplay() {
    logAvailableInstanceExtensions();
    updateAvailableValidationLayerNames();
    createVKInstance();
    createVKDebugReportCallback();
    createVKAndroidSurface();
    selectPhysicalDevice();
    logSelectedPhysicalDeviceProperties();
    updatePhysicalDeviceFeatures();
    logSelectedPhysicalDeviceAvailableExtensions();
    updatePhysicalDeviceSurfaceCapabilities();
    updatePhysicalDeviceGraphicsQueueFamilyIndex();
    checkSelectedPhysicalDeviceGraphicsQueueSurfaceSupport();
    selectPhysicalDeviceSurfaceFormat();
    selectPhysicalDeviceSurfacePresentMode();
    createLogicalDevice();
    vkGetDeviceQueue(vkDevice, physicalDeviceGraphicsQueueFamilyIndex, 0, &graphicsQueue);
    createSwapChain();
    createRenderPass();
    createGraphicsPipeline();


    // create frame buffers
    swapChainFrameBuffers.resize(swapChainImageViews.size());
    for(size_t i = 0; i < swapChainFrameBuffers.size(); i++) {
        VkImageView attachments[] = {
                swapChainImageViews[i]
        };
        VkFramebufferCreateInfo framebufferCreateInfo{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderPass = renderPass,
                .attachmentCount = 1,
                .pAttachments = attachments,
                .width = physicalDeviceSurfaceCapabilities.currentExtent.width,
                .height = physicalDeviceSurfaceCapabilities.currentExtent.height,
                .layers = 1
        };
        vkCreateFramebuffer(vkDevice, &framebufferCreateInfo, nullptr,
                            &swapChainFrameBuffers[i]);
    }

    // create command pool
    VkCommandPoolCreateInfo commandPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = physicalDeviceGraphicsQueueFamilyIndex,
            .flags = 0,
            .pNext = nullptr,
    };
    vkCreateCommandPool(vkDevice, &commandPoolCreateInfo, nullptr, &commandPool);

    // allocate command buffer
    commandBuffers.resize(swapChainFrameBuffers.size());
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32_t>(commandBuffers.size()),
    };
    vkAllocateCommandBuffers(vkDevice, &commandBufferAllocateInfo,
                             commandBuffers.data());

    // record command buffer
    for(size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo commandBufferBeginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
                .pInheritanceInfo = nullptr,
        };
        vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo);

        VkClearValue clearColor;
        clearColor.color.float32[0] = 0.0f;
        clearColor.color.float32[1] = 0.0f;
        clearColor.color.float32[2] = 0.0f;
        clearColor.color.float32[3] = 1.0f;
        VkRenderPassBeginInfo renderPassBeginInfo{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .pNext = nullptr,
                .renderPass = renderPass,
                .framebuffer = swapChainFrameBuffers[i],
                .renderArea.offset = {0, 0},
                .renderArea.extent = physicalDeviceSurfaceCapabilities.currentExtent,
                .clearValueCount = 1,
                .pClearValues = &clearColor,
        };
        PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
        vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)vkGetInstanceProcAddr(
                vkInstance, "vkCmdBeginRenderPass");
        vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                          graphicsPipeline);

        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);
        vkEndCommandBuffer(commandBuffers[i]);
    }

    // create sync objects
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    VkSemaphoreCreateInfo semaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
    };
    VkFenceCreateInfo fenceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkCreateSemaphore(vkDevice, &semaphoreCreateInfo, nullptr,
                          &imageAvailableSemaphores[i]);
        vkCreateSemaphore(vkDevice, &semaphoreCreateInfo, nullptr,
                          &renderFinishedSemaphores[i]);
        vkCreateFence(vkDevice, &fenceCreateInfo, nullptr, &inFlightFences[i]);
    }

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
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
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

    vkDestroyCommandPool(vkDevice, commandPool, nullptr);

    for(auto framebuffer: swapChainFrameBuffers) {
        vkDestroyFramebuffer(vkDevice, framebuffer, nullptr);
    }

    vkDestroyPipeline(vkDevice, graphicsPipeline, nullptr);
    vkDestroyRenderPass(vkDevice, renderPass, nullptr);
    vkDestroyPipelineLayout(vkDevice, graphicsPipelineLayout, nullptr);

    for(auto imageView: swapChainImageViews) {
        vkDestroyImageView(vkDevice, imageView, nullptr);
    }
    vkDestroySwapchainKHR(vkDevice, vkSwapchain, nullptr);
    vkDestroyDevice(vkDevice, nullptr);
    vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);

    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
    vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)
            vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
    vkDestroyDebugReportCallbackEXT(vkInstance, vkDebugReportCallbackExt, nullptr);

    vkDestroyInstance(vkInstance, nullptr);
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

