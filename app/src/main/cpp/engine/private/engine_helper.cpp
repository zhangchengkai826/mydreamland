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
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayerNames->size());
    instanceCreateInfo.ppEnabledLayerNames = validationLayerNames->data();
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
    deviceCreateInfo.enabledLayerCount = validationLayerNames->size();
    deviceCreateInfo.ppEnabledLayerNames = validationLayerNames->data();
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
    vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &imageCount,
                            swapChainImages);

    // create swap chain image views
    for(int i = 0; i < NUM_IMAGES_IN_SWAPCHAIN; i++) {
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
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
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
    for(int i = 0; i < NUM_IMAGES_IN_SWAPCHAIN; i++) {
        createImage(physicalDeviceSurfaceCapabilities.currentExtent.width,
                    physicalDeviceSurfaceCapabilities.currentExtent.height, 1,
                    VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    depthStencilImages[i], depthStencilImageMemorys[i]);
        depthStencilImageViews[i] = createImageView(depthStencilImages[i],
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 1);
    }
}

void Engine::createFrameBuffers() {
    for(size_t i = 0; i < NUM_IMAGES_IN_SWAPCHAIN; i++) {
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
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = physicalDeviceGraphicsQueueFamilyIndex,
    };
    vkCreateCommandPool(vkDevice, &commandPoolCreateInfo, nullptr, &commandPool);
}

void Engine::recordFrameCmdBuffers(int imageIndex) {
    VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr,
    };
    /* implicitly reset command buffer */
    vkBeginCommandBuffer(frameCommandBuffers[currentFrame], &beginInfo);

    std::array<VkClearValue, 2> clearValues{{
        {.color = {.float32 = {0.0f, 1.0f, 0.0f, 1.0f}}},
        {.depthStencil = {.depth = 1.0f, .stencil = 0}},
    }};
    VkRenderPassBeginInfo renderPassBeginInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = renderPass,
            .framebuffer = swapChainFrameBuffers[imageIndex],
            .renderArea.offset = {0, 0},
            .renderArea.extent = physicalDeviceSurfaceCapabilities.currentExtent,
            .clearValueCount = static_cast<uint32_t>(clearValues.size()),
            .pClearValues = clearValues.data(),
    };
    PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
    vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)vkGetInstanceProcAddr(
            vkInstance, "vkCmdBeginRenderPass");
    vkCmdBeginRenderPass(frameCommandBuffers[currentFrame], &renderPassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    std::array<VkDescriptorSet, 1> frameDescriptorSets =
            {staticDescriptorSets[currentFrame]};
    vkCmdBindDescriptorSets(frameCommandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, 0, static_cast<uint32_t>(frameDescriptorSets.size()),
                            frameDescriptorSets.data(), 0, nullptr);

    pthread_mutex_lock(&mutex);
    for(auto it = object3ds->begin(); it != object3ds->end(); it++) {
        VkBuffer vertexBuffers[] = {it->second.geo->vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(frameCommandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(frameCommandBuffers[currentFrame], it->second.geo->indexBuffer, 0,
                             VK_INDEX_TYPE_UINT16);

        /* note that vkCmdPushConstants.pValues is instantly remembered by the command buffer, and if
         * the data pointed by pValues changes afterwords, it has no effect on command buffer
         */
        vkCmdPushConstants(frameCommandBuffers[currentFrame], pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT, 0, 64, &it->second.modelMat);
        vkCmdPushConstants(frameCommandBuffers[currentFrame], pipelineLayout,
                           VK_SHADER_STAGE_FRAGMENT_BIT, 64, 4, &it->second.texId);

        vkCmdBindPipeline(frameCommandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                          (*pipelines)["3d"]);

        vkCmdDrawIndexed(frameCommandBuffers[currentFrame], static_cast<uint32_t>(
                it->second.geo->nIndices), 1, 0, 0, 0);
    }
    pthread_mutex_unlock(&mutex);

    for(auto it = object2ds->begin(); it != object2ds->end(); it++) {
        VkBuffer vertexBuffers[] = {it->second.geo->vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(frameCommandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(frameCommandBuffers[currentFrame], it->second.geo->indexBuffer, 0,
                             VK_INDEX_TYPE_UINT16);

        vkCmdPushConstants(frameCommandBuffers[currentFrame], pipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT, 0, 64, &it->second.modelMat);
        vkCmdPushConstants(frameCommandBuffers[currentFrame], pipelineLayout,
                           VK_SHADER_STAGE_FRAGMENT_BIT, 64, 4, &it->second.texId);

        vkCmdBindPipeline(frameCommandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                          (*pipelines)["2d"]);

        vkCmdDrawIndexed(frameCommandBuffers[currentFrame], static_cast<uint32_t>(
                it->second.geo->nIndices), 1, 0, 0, 0);

    }

    vkCmdEndRenderPass(frameCommandBuffers[currentFrame]);
    vkEndCommandBuffer(frameCommandBuffers[currentFrame]);
}

void Engine::createSamplers() {
    VkSamplerCreateInfo samplerCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = 16,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .mipLodBias = 0.0f,
            .minLod = 0.0f,
            .maxLod = 0.0f,
    };
    vkCreateSampler(vkDevice, &samplerCreateInfo, nullptr, &sampler);
}

void Engine::createDescriptorSetLayouts() {
    std::array<VkDescriptorSetLayoutBinding, 2> staticSetLayoutBinding{{
        {.binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr,},
        {.binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = MAX_TEXTURES_PER_FRAME,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr,},
    }};
    VkDescriptorSetLayoutCreateInfo staticSetLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = static_cast<uint32_t>(staticSetLayoutBinding.size()),
            .pBindings = staticSetLayoutBinding.data(),
    };
    vkCreateDescriptorSetLayout(vkDevice, &staticSetLayoutCreateInfo, nullptr,
                                &staticSetLayout);
}

void Engine::createDescriptorPools() {
    std::array<VkDescriptorPoolSize, 2> staticPoolSize = {{
        {.descriptorCount = MAX_FRAMES_IN_FLIGHT,
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,},
        {.descriptorCount = MAX_FRAMES_IN_FLIGHT * MAX_TEXTURES_PER_FRAME,
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,}
    }};
    VkDescriptorPoolCreateInfo staticPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .poolSizeCount = static_cast<uint32_t>(staticPoolSize.size()),
            .pPoolSizes = staticPoolSize.data(),
            .maxSets = MAX_FRAMES_IN_FLIGHT,
    };
    vkCreateDescriptorPool(vkDevice, &staticPoolCreateInfo, nullptr, &staticDescriptorPool);

    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        layouts[i] = staticSetLayout;
    };
    VkDescriptorSetAllocateInfo staticSetsAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = staticDescriptorPool,
            .descriptorSetCount = MAX_FRAMES_IN_FLIGHT,
            .pSetLayouts = layouts,
    };
    vkAllocateDescriptorSets(vkDevice, &staticSetsAllocateInfo, staticDescriptorSets);
}

void Engine::prefillStaticSets() {
    uint32_t bufferStride = sizeof(glm::mat4);
    uint32_t bufferSize = bufferStride * MAX_FRAMES_IN_FLIGHT;
    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 uniformBuffer, uniformBuffersMemory);

    glm::mat4 V = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 P = glm::perspective(glm::radians(45.0f),
            physicalDeviceSurfaceCapabilities.currentExtent.width /
            static_cast<float>(physicalDeviceSurfaceCapabilities.currentExtent.height),
            0.1f, 10.0f);
    /* Vulkan NDC y-axis points downwards, while OpenGL's pointing upwards.
     * Both of their NDC z-axis points inwards, so Vulkan is right-handed while OpenGL left-handed.
     * Glm is designed for OpenGL, and it's generated PVM matrices will eventually put points into
     * OpenGL's NDC space. So suppose we use glm to transform a vertex X into NDC (-0.2, 0.5, 0.2).
     * If we are using OpenGL, X will appear in the upper half of the screen, which is what we expected.
     * But if we are using Vulkan, X will appear in the lower half of the screen, as if we make the camera
     * upside-down. To solve this problem, in Vulkan, we adjust P matrix to flip the NDC y value of all vertices.
     * Because we flip one axis, we should also flip the front dace definition
     * from CLOCKWISE(left-handed, OpenGL convention) to COUNTER-CLOCKWISE (right-handed, Vulkan convention).
     * (Here we assume glm-generated PVM matrices operates on OpenGL conventional clockwise-as-front-face 3d models.)
     *
     * Both Vulkan & OpenGL's NDC range is [x/y: -1 ~ 1, z: 0 ~ 1]
     * Vertices with NDC outside this range is clipped.
     */
    P[1][1] *= -1;
    glm::mat4 PV = P * V;

    void *data;
    vkMapMemory(vkDevice, uniformBuffersMemory, 0, bufferSize, 0, &data);
    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        memcpy(reinterpret_cast<uint8_t *>(data) + bufferStride, &PV, sizeof(PV));
    }
    vkUnmapMemory(vkDevice, uniformBuffersMemory);

    std::vector<VkWriteDescriptorSet> writeDescriptorSets;
    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{
                .buffer = uniformBuffer,
                .offset = bufferStride * i,
                .range = bufferStride,
        };
        VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = staticDescriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo = &bufferInfo,
            .pImageInfo = nullptr,
            .pTexelBufferView = nullptr,
        };
        writeDescriptorSets.push_back(write);
    }

    std::vector<VkImageView> views;
    int texId = -1;

    for(auto it = object3ds->begin(); it != object3ds->end(); it++) {
        VkImageView v = it->second.tex->imageView;
        if(std::find(views.cbegin(), views.cend(), v) == views.cend()) {
            views.push_back(v);
            texId++;
        }
        it->second.texId = texId;
    }

    for(auto it = object2ds->begin(); it != object2ds->end(); it++) {
        VkImageView v = it->second.tex->imageView;
        if(std::find(views.cbegin(), views.cend(), v) == views.cend()) {
            views.push_back(v);
            texId++;
        }
        it->second.texId = texId;
    }

    std::vector<VkDescriptorImageInfo> imageInfo;
    for(int i = 0; i < views.size(); i++) {
        VkDescriptorImageInfo info{
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .imageView = views[i],
                .sampler = sampler,
        };
        imageInfo.push_back(info);
    }
    for(int i = views.size(); i < MAX_TEXTURES_PER_FRAME; i++) {
        VkDescriptorImageInfo info{
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .imageView = views[0],
                .sampler = sampler,
        };
        imageInfo.push_back(info);
    }
    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkWriteDescriptorSet write{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = staticDescriptorSets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = static_cast<uint32_t>(imageInfo.size()),
                .pBufferInfo = nullptr,
                .pImageInfo = imageInfo.data(),
                .pTexelBufferView = nullptr,
        };
        writeDescriptorSets.push_back(write);
    }

    vkUpdateDescriptorSets(vkDevice, static_cast<uint32_t>(writeDescriptorSets.size()),
                           writeDescriptorSets.data(), 0, nullptr);
}

void Engine::createFrameSyncObjs() {
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
        vkCreateFence(vkDevice, &fenceCreateInfo, nullptr, &inFlightFrameFences[i]);
    }
}

VkCommandBuffer Engine::beginOneTimeSubmitCommands() {
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

void Engine::endOneTimeSubmitCommandsSyncWithFence(VkCommandBuffer commandBuffer) {
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

void Engine::allocFrameCmdBuffers() {
    VkCommandBufferAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = commandPool,
            .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    };
    vkAllocateCommandBuffers(vkDevice, &allocateInfo, frameCommandBuffers);
}