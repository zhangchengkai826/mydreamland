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
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .pApplicationName = "mydreamland",
            .pEngineName = "mydreamland_engine",
    };

    std::vector<const char *> instanceExt;
    instanceExt.push_back("VK_KHR_surface");
    instanceExt.push_back("VK_KHR_android_surface");
    if(DEBUG_ON && validationLayerNames.size() > 0) {
        instanceExt.push_back("VK_EXT_debug_report");
    }

    VkInstanceCreateInfo instanceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .pApplicationInfo = &appInfo,
            .enabledExtensionCount = static_cast<uint32_t>(instanceExt.size()),
            .ppEnabledExtensionNames = instanceExt.data(),
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
    };
    if(DEBUG_ON && validationLayerNames.size() > 0) {
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayerNames.size());
        instanceCreateInfo.ppEnabledLayerNames = validationLayerNames.data();
    }
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
    if(DEBUG_ON && validationLayerNames.size() > 0) {
        deviceCreateInfo.enabledLayerCount = validationLayerNames.size();
        deviceCreateInfo.ppEnabledLayerNames = validationLayerNames.data();
    }
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
        VkImageViewCreateInfo imgViewCreateInfo {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = swapChainImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = physicalDeviceSurfaceFormat.format,
                .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
                .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .subresourceRange.baseMipLevel = 0,
                .subresourceRange.levelCount = 1,
                .subresourceRange.baseArrayLayer = 0,
                .subresourceRange.layerCount = 1,
        };
        vkCreateImageView(vkDevice, &imgViewCreateInfo, nullptr,
                          &swapChainImageViews[i]);
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

    VkAttachmentReference colorAttachmentRef{
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpassDescription{
            .flags = 0,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr,
            .pResolveAttachments = nullptr,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    };

    VkRenderPassCreateInfo renderPassCreateInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount = 1,
            .pAttachments = &colorAttachmentDescription,
            .subpassCount = 1,
            .pSubpasses = &subpassDescription,
            .dependencyCount = 0,
            .pDependencies = nullptr,
    };
    vkCreateRenderPass(vkDevice, &renderPassCreateInfo, nullptr, &renderPass);
}

void Engine::createGraphicsPipeline() {
    // create graphics pipeline
    auto vertShaderCode = readFile("shaders/shader.vert.spv");
    auto fragShaderCode = readFile("shaders/shader.frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertShaderModule,
            .pName = "main",
            .flags = 0,
            .pSpecializationInfo = nullptr,
    };
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragShaderModule,
            .pName = "main",
            .flags = 0,
            .pSpecializationInfo = nullptr,
    };
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data(),
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport viewport{
            .x = 0,
            .y = 0,
            .width = static_cast<float>(physicalDeviceSurfaceCapabilities.currentExtent.width),
            .height = static_cast<float>(physicalDeviceSurfaceCapabilities.currentExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
    };
    VkRect2D scissor{
            .offset = {0, 0},
            .extent = physicalDeviceSurfaceCapabilities.currentExtent,
    };
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor,
    };

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .lineWidth = 1.0f,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE, //
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .sampleShadingEnable = VK_FALSE,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            .blendEnable = VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
    };

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachmentState,
            .blendConstants[0] = 0.0f,
            .blendConstants[1] = 0.0f,
            .blendConstants[2] = 0.0f,
            .blendConstants[3] = 0.0f,
    };

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputStateCreateInfo,
            .pInputAssemblyState = &inputAssemblyStateCreateInfo,
            .pViewportState = &viewportStateCreateInfo,
            .pRasterizationState = &rasterizationStateCreateInfo,
            .pMultisampleState = &multisampleStateCreateInfo,
            .pDepthStencilState = nullptr,
            .pColorBlendState = &colorBlendStateCreateInfo,
            .pDynamicState = nullptr,
            .layout = graphicsPipelineLayout,
            .renderPass = renderPass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1,
    };
    vkCreateGraphicsPipelines(vkDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr,
                              &graphicsPipeline);

    vkDestroyShaderModule(vkDevice, vertShaderModule, nullptr);
    vkDestroyShaderModule(vkDevice, fragShaderModule, nullptr);
}

void Engine::createFrameBuffers() {
    swapChainFrameBuffers.resize(NUM_IMAGES_IN_SWAPCHAIN);
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

        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                graphicsPipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

        vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

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

void Engine::createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(vkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(vkDevice, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(vkDevice, stagingBufferMemory, nullptr);
}

void Engine::createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(uint16_t) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(vkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(vkDevice, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(vkDevice, stagingBufferMemory, nullptr);
}


void Engine::createGraphicsPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = 1,
            .pSetLayouts = &descriptorSetLayout,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
    };
    vkCreatePipelineLayout(vkDevice, &pipelineLayoutCreateInfo, nullptr,
                           &graphicsPipelineLayout);
}

void Engine::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(UniformBuffer);
    uniformBuffers.resize(NUM_IMAGES_IN_SWAPCHAIN);
    uniformBuffersMemory.resize(NUM_IMAGES_IN_SWAPCHAIN);
    for(size_t i = 0; i < NUM_IMAGES_IN_SWAPCHAIN; i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                uniformBuffers[i], uniformBuffersMemory[i]);
    }
}

void Engine::updateUniformBuffer(uint32_t imageIndex) {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float,
        std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBuffer ubo{
        .model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                glm::vec3(0.0f, 0.0f, 1.0f)),
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

    void *data;
    vkMapMemory(vkDevice, uniformBuffersMemory[imageIndex], 0, sizeof(UniformBuffer), 0, &data);
    memcpy(data, &ubo, sizeof(UniformBuffer));
    vkUnmapMemory(vkDevice, uniformBuffersMemory[imageIndex]);
}

void Engine::createDescriptorPool() {
    VkDescriptorPoolSize poolSize{
        .descriptorCount = NUM_IMAGES_IN_SWAPCHAIN,
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    };
    VkDescriptorPoolCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize,
        .maxSets = static_cast<uint32_t>(NUM_IMAGES_IN_SWAPCHAIN),
    };

    vkCreateDescriptorPool(vkDevice, &createInfo, nullptr, &descriptorPool);
}

void Engine::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding binding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr,
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = 1,
            .pBindings = &binding,
    };
    vkCreateDescriptorSetLayout(vkDevice, &descriptorSetLayoutCreateInfo, nullptr,
                                &descriptorSetLayout);
}

void Engine::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(NUM_IMAGES_IN_SWAPCHAIN, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 3,
        .pSetLayouts = layouts.data(),
    };
    descriptorSets.resize(NUM_IMAGES_IN_SWAPCHAIN);
    vkAllocateDescriptorSets(vkDevice, &allocateInfo, descriptorSets.data());

    for(size_t i = 0; i < NUM_IMAGES_IN_SWAPCHAIN; i++) {
        VkDescriptorBufferInfo bufferInfo{
            .buffer = uniformBuffers[i],
            .offset = 0,
            .range = sizeof(UniformBuffer),
        };
        VkWriteDescriptorSet writeDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = descriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo = &bufferInfo,
            .pImageInfo = nullptr,
            .pTexelBufferView = nullptr,
        };
        vkUpdateDescriptorSets(vkDevice, 1, &writeDescriptorSet, 0, nullptr);
    }
}
