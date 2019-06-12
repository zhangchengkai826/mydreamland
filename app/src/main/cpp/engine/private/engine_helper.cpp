//
// Created by andys on 6/2/2019.
//

#include <engine.h>

void Engine::createVKInstance() {
    VkApplicationInfo appInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .apiVersion = VK_MAKE_VERSION(1, 0, 66),
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pApplicationName = "mydreamland",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "mydreamland_engine",
    };

    std::vector<const char *> instanceExt;
    instanceExt.push_back("VK_KHR_surface");
    instanceExt.push_back("VK_KHR_android_surface");

#ifdef DEBUG
    instanceExt.push_back("VK_EXT_debug_report");
#endif

    VkInstanceCreateInfo instanceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .pApplicationInfo = &appInfo,
            .enabledExtensionCount = static_cast<uint32_t>(instanceExt.size()),
            .ppEnabledExtensionNames = instanceExt.data(),
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
    };

#ifdef DEBUG
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayerNames.size());
    instanceCreateInfo.ppEnabledLayerNames = validationLayerNames.data();
#endif

    vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance);
}

void Engine::createVKAndroidSurface() {
    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfoKhr{
            .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .window = window};
    vkCreateAndroidSurfaceKHR(vkInstance, &surfaceCreateInfoKhr, nullptr,
                              &vkSurface);
}

void Engine::selectPhysicalDevice() {
    // on Android, every GPU (physical device) is equal -- supporting graphics/compute/present
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);
    assert(physicalDeviceCount > 0);
    VkPhysicalDevice physicalDevices[physicalDeviceCount];
    vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevices);
    vkPhysicalDevice = physicalDevices[0];  // Pick up the first physical device
}

void Engine::selectPhysicalDeviceSurfaceFormat() {
    physicalDeviceSurfaceFormat = {VK_FORMAT_R8G8B8A8_UNORM,
                                   VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatCount, nullptr);
    assert(formatCount > 0);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatCount,
                                         formats.data());
    bool b_formatSupport = false;
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "Vulkan Selected Physical Device Surface Supported Formats:");
    for(const auto &format: formats) {
        if(format.format == physicalDeviceSurfaceFormat.format && format.colorSpace ==
                                                                  physicalDeviceSurfaceFormat.colorSpace) {
            b_formatSupport = true;
        }
        if(format.format == VK_FORMAT_UNDEFINED) {
            b_formatSupport = true;
        }
        __android_log_print(ANDROID_LOG_INFO, "main",
                            "\tFormat: %d\tColor Space: %d", format.format, format.colorSpace);
    }
    assert(b_formatSupport);
}

void Engine::selectPhysicalDeviceSurfacePresentMode() {
    physicalDeviceSurfacePresentMode = VK_PRESENT_MODE_FIFO_KHR;

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentModeCount,
                                              nullptr);
    assert(presentModeCount > 0);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentModeCount,
                                              presentModes.data());
    bool b_presentModeSupport = false;
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "Vulkan Selected Physical Device Surface Supported Present Modes:");
    for(const auto &pm: presentModes) {
        if(pm == physicalDeviceSurfacePresentMode) {
            b_presentModeSupport = true;
        }
        __android_log_print(ANDROID_LOG_INFO, "main", "\tPresent Mode: %d", pm);
    }
    assert(b_presentModeSupport);
}

void Engine::updatePhysicalDeviceFeatures() {
    vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &physicalDeviceFeatures);
    __android_log_print(ANDROID_LOG_INFO, "main", "Vulkan Selected Physical Device Features:");
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "Support Tessellation Shader: %d",
                        physicalDeviceFeatures.tessellationShader);
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "Support Geometry Shader: %d", physicalDeviceFeatures.geometryShader);
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "Support Index32: %d", physicalDeviceFeatures.fullDrawIndexUint32);
}

void Engine::updatePhysicalDeviceSurfaceCapabilities() {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, vkSurface,
                                              &physicalDeviceSurfaceCapabilities);

    __android_log_print(ANDROID_LOG_INFO, "main",
            "Vulkan Selected Physical Device Surface Capabilities:");
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "\timage count: %u - %u", physicalDeviceSurfaceCapabilities.minImageCount,
                        physicalDeviceSurfaceCapabilities.maxImageCount);
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "\tarray layers: %u",
                        physicalDeviceSurfaceCapabilities.maxImageArrayLayers);
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "\timage size (now): %dx%d",
                        physicalDeviceSurfaceCapabilities.currentExtent.width,
                        physicalDeviceSurfaceCapabilities.currentExtent.height);
    __android_log_print(ANDROID_LOG_INFO, "main", "\timage size (extent): %dx%d - %dx%d",
                        physicalDeviceSurfaceCapabilities.minImageExtent.width,
                        physicalDeviceSurfaceCapabilities.minImageExtent.height,
                        physicalDeviceSurfaceCapabilities.maxImageExtent.width,
                        physicalDeviceSurfaceCapabilities.maxImageExtent.height);
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "\tusage: %x", physicalDeviceSurfaceCapabilities.supportedUsageFlags);
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "\tcurrent transform: %u",
                        physicalDeviceSurfaceCapabilities.currentTransform);
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "\tallowed transforms: %x",
                        physicalDeviceSurfaceCapabilities.supportedTransforms);
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "\tcomposite alpha flags: %u",
                        physicalDeviceSurfaceCapabilities.supportedCompositeAlpha);
}

void Engine::updatePhysicalDeviceGraphicsQueueFamilyIndex() {
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);
    assert(queueFamilyCount);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount,
                                             queueFamilyProperties.data());

    for (physicalDeviceGraphicsQueueFamilyIndex = 0;
    physicalDeviceGraphicsQueueFamilyIndex < queueFamilyCount;
    physicalDeviceGraphicsQueueFamilyIndex++) {
        if (queueFamilyProperties[physicalDeviceGraphicsQueueFamilyIndex].queueFlags
        & VK_QUEUE_GRAPHICS_BIT) {
            break;
        }
    }
    assert(physicalDeviceGraphicsQueueFamilyIndex < queueFamilyCount);
}

void Engine::createLogicalDevice() {
    float priorities[] = {
            1.0f,
    };
    VkDeviceQueueCreateInfo queueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCount = 1,
            .queueFamilyIndex = physicalDeviceGraphicsQueueFamilyIndex,
            .pQueuePriorities = priorities,
    };
    std::vector<const char *> deviceExt;
    deviceExt.push_back("VK_KHR_swapchain");
    VkDeviceCreateInfo deviceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueCreateInfo,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<uint32_t>(deviceExt.size()),
            .ppEnabledExtensionNames = deviceExt.data(),
            .pEnabledFeatures = &physicalDeviceFeatures,
    };

#ifdef DEBUG
    deviceCreateInfo.enabledLayerCount = validationLayerNames.size();
    deviceCreateInfo.ppEnabledLayerNames = validationLayerNames.data();
#endif

    vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, nullptr, &vkDevice);
}

void Engine::createSwapChain() {
    assert(physicalDeviceSurfaceCapabilities.minImageCount > 1);
    uint32_t imageCount = physicalDeviceSurfaceCapabilities.minImageCount;
    if(imageCount + 1 <= physicalDeviceSurfaceCapabilities.maxImageCount) {
        imageCount += 1;
    }
    assert(imageCount == NUM_IMAGES_IN_SWAPCHAIN);

    VkSwapchainCreateInfoKHR swapchainCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = vkSurface,
            .minImageCount = NUM_IMAGES_IN_SWAPCHAIN,
            .imageFormat = physicalDeviceSurfaceFormat.format,
            .imageColorSpace = physicalDeviceSurfaceFormat.colorSpace,
            .imageExtent = physicalDeviceSurfaceCapabilities.currentExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
            .presentMode = physicalDeviceSurfacePresentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE,
    };
    vkCreateSwapchainKHR(vkDevice, &swapchainCreateInfo, nullptr, &vkSwapchain);

    // retrieve swap chain images
    vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &imageCount, nullptr);
    swapChainImages.resize(NUM_IMAGES_IN_SWAPCHAIN);
    vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &imageCount,
                            swapChainImages.data());

    // create swap chain image views
    swapChainImageViews.resize(NUM_IMAGES_IN_SWAPCHAIN);
    for(int i = 0; i < swapChainImageViews.size(); i++) {
        swapChainImageViews[i] = createImageView(swapChainImages[i],
                                                 physicalDeviceSurfaceFormat.format,
                                                 VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void Engine::createRenderPass() {
    VkAttachmentDescription colorAttachmentDescription{
            .format = physicalDeviceSurfaceFormat.format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .flags = 0,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    VkAttachmentDescription depthStencilAttachmentDescription{
            .format = VK_FORMAT_D24_UNORM_S8_UINT,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .flags = 0,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachmentDescription,
                                                          depthStencilAttachmentDescription};

    VkAttachmentReference colorAttachmentRef{
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkAttachmentReference depthStencilAttachmentRef{
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpassDescription{
            .flags = 0,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .pDepthStencilAttachment = &depthStencilAttachmentRef,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr,
            .pResolveAttachments = nullptr,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    };

    /* WAR data hazard, only needs execution dependency (presentation <- layout transition) */
    VkSubpassDependency dependencyExternalTo0{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        /* sub-pass guarantees execution dependency
         * (layout transition <- sub-pass who needs that layout)
         */
        .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .dstAccessMask = 0,
        .dependencyFlags = 0,
    };

    /* VkSubmitInfo.pSignalSemaphores takes care of everything */
    VkSubpassDependency dependency0ToExternal{
        .srcSubpass = 0,
        .dstSubpass = VK_SUBPASS_EXTERNAL,
        .srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .dstAccessMask = 0,
        .dependencyFlags = 0,
    };

    VkSubpassDependency dependencies[] = {dependencyExternalTo0, dependency0ToExternal};

    VkRenderPassCreateInfo renderPassCreateInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount = 2,
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpassDescription,
            .dependencyCount = 2,
            .pDependencies = dependencies,
    };
    vkCreateRenderPass(vkDevice, &renderPassCreateInfo, nullptr, &renderPass);
}

void Engine::createDepthStencilResources() {
    depthStencilImageMemorys.resize(NUM_IMAGES_IN_SWAPCHAIN);
    depthStencilImages.resize(NUM_IMAGES_IN_SWAPCHAIN);
    depthStencilImageViews.resize(NUM_IMAGES_IN_SWAPCHAIN);
    for(int i = 0; i < NUM_IMAGES_IN_SWAPCHAIN; i++) {
        createImage(physicalDeviceSurfaceCapabilities.currentExtent.width,
                    physicalDeviceSurfaceCapabilities.currentExtent.height, 1,
                    VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    depthStencilImages[i], depthStencilImageMemorys[i]);
        depthStencilImageViews[i] = createImageView(depthStencilImages[i], VK_FORMAT_D24_UNORM_S8_UINT,
                                                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                                                1);
    }
}

void Engine::createFrameBuffers() {
    swapChainFrameBuffers.resize(NUM_IMAGES_IN_SWAPCHAIN);
    for(size_t i = 0; i < swapChainFrameBuffers.size(); i++) {
        std::array<VkImageView, 2> attachments = {
                swapChainImageViews[i],
                depthStencilImageViews[i],
        };
        VkFramebufferCreateInfo framebufferCreateInfo{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderPass = renderPass,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .width = physicalDeviceSurfaceCapabilities.currentExtent.width,
                .height = physicalDeviceSurfaceCapabilities.currentExtent.height,
                .layers = 1
        };
        vkCreateFramebuffer(vkDevice, &framebufferCreateInfo, nullptr,
                            &swapChainFrameBuffers[i]);
    }
}

void Engine::createCmdPool() {
    VkCommandPoolCreateInfo commandPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = physicalDeviceGraphicsQueueFamilyIndex,
            .flags = 0,
            .pNext = nullptr,
    };
    vkCreateCommandPool(vkDevice, &commandPoolCreateInfo, nullptr, &commandPool);
}

void Engine::allocCmdBuffers() {
    commandBuffers.resize(NUM_IMAGES_IN_SWAPCHAIN);
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32_t>(commandBuffers.size()),
    };
    vkAllocateCommandBuffers(vkDevice, &commandBufferAllocateInfo,
                             commandBuffers.data());
}

void Engine::recordCmdBuffers() {
    for(size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo commandBufferBeginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .pNext = nullptr,
                .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
                .pInheritanceInfo = nullptr,
        };
        vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo);

        std::array<VkClearValue, 2> clearValues{{
            {.color = {.float32 = {0.0f, 0.0f, 0.0f, 1.0f}}},
            {.depthStencil = {.depth = 1.0f, .stencil = 0}},
        }};
        VkRenderPassBeginInfo renderPassBeginInfo{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .pNext = nullptr,
                .renderPass = renderPass,
                .framebuffer = swapChainFrameBuffers[i],
                .renderArea.offset = {0, 0},
                .renderArea.extent = physicalDeviceSurfaceCapabilities.currentExtent,
                .clearValueCount = static_cast<uint32_t>(clearValues.size()),
                .pClearValues = clearValues.data(),
        };
        PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
        vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)vkGetInstanceProcAddr(
                vkInstance, "vkCmdBeginRenderPass");
        vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                          material.graphicsPipeline);

        VkBuffer vertexBuffers[] = {geometry.vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffers[i], geometry.indexBuffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                material.graphicsPipelineLayout, 0, 1, &material.descriptorSet,
                                0, nullptr);

        vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(geometry.nIndices), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);
        vkEndCommandBuffer(commandBuffers[i]);
    }
}

void Engine::createSyncObjs() {
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
}


void Engine::createUniformBuffers() {
    VkCommandBuffer commandBuffer = beginOneTimeSubmitCommands();

    VkDeviceSize bufferSize = sizeof(UniformBuffer);
    UniformBuffer ubo{
            .model = glm::mat4(1.0f),
            .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f)),
            .proj = glm::perspective(glm::radians(45.0f),
                    physicalDeviceSurfaceCapabilities.currentExtent.width /
                    static_cast<float>(physicalDeviceSurfaceCapabilities.currentExtent.height),
                    0.1f, 10.0f),
    };
    /* Vulkan NDC y-axis points downwards, while OpenGL's pointing upwards
     *
     * projection matrix determines handedness, so Vulkan is right-handedness instead of OpenGL's
     * left-handedness
     */
    ubo.proj[1][1] *= -1;

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                uniformBuffer, uniformBuffersMemory);

    vkCmdUpdateBuffer(commandBuffer, uniformBuffer, 0, bufferSize, &ubo);

    endOneTimeSubmitCommandsSyncWithFence(commandBuffer);
}



VkCommandBuffer Engine::beginOneTimeSubmitCommands() const {
    VkCommandBufferAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = commandPool,
            .commandBufferCount = 1,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    };
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(vkDevice, &allocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr,
    };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void Engine::endOneTimeSubmitCommandsSyncWithFence(VkCommandBuffer commandBuffer) const {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .pWaitDstStageMask = nullptr,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr,
    };

    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };
    vkCreateFence(vkDevice, &fenceCreateInfo, nullptr, &fence);

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence);
    /* fence signal & wait will make all memory access available & visible */
    vkWaitForFences(vkDevice, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

    vkDestroyFence(vkDevice, fence, nullptr);
    vkFreeCommandBuffers(vkDevice, commandPool, 1, &commandBuffer);
}

void Engine::loadResources() {
    VkCommandBuffer commandBuffer = beginOneTimeSubmitCommands();

    geometry.initFromFile(this, commandBuffer,
            "/storage/emulated/0/Documents/mydreamland/geometry/vertices.dat");
    texture.initFromFile(this, commandBuffer,
            "/storage/emulated/0/Documents/mydreamland/texture/texture.jpg");
    material.init(this, &texture);

    endOneTimeSubmitCommandsSyncWithFence(commandBuffer);
}

void Engine::destroyResources() {
    material.destroy(this);
    texture.destroy(this);
    geometry.destroy(this);
}