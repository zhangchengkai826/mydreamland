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

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachmentDescription,
                                                          depthStencilAttachmentDescription};
    VkRenderPassCreateInfo renderPassCreateInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount = 2,
            .pAttachments = attachments.data(),
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

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .minDepthBounds = 0.0f,
            .maxDepthBounds = 1.0f,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
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
            .pDepthStencilState = &depthStencilStateCreateInfo,
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
        std::array<VkImageView, 2> attachments = {
                swapChainImageViews[i],
                depthStencilImageView
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
                          graphicsPipeline);

        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                graphicsPipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

        vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(geometry.indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);
        vkEndCommandBuffer(commandBuffers[i]);
    }
}

void Engine::createSyncObjs() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
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
        vkCreateFence(vkDevice, &fenceCreateInfo, nullptr, &inFlightFences[i]);
    }
}

void Engine::createVertexBuffer() {
    VkDeviceSize bufferSize = sizeof(Vertex) * geometry.vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(vkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, geometry.vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(vkDevice, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(vkDevice, stagingBufferMemory, nullptr);
}

void Engine::createIndexBuffer() {
    VkDeviceSize bufferSize = sizeof(uint16_t) * geometry.indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(vkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, geometry.indices.data(), static_cast<size_t>(bufferSize));
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

void Engine::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes = {{
            {.descriptorCount = NUM_IMAGES_IN_SWAPCHAIN,
              .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,},
            {.descriptorCount = NUM_IMAGES_IN_SWAPCHAIN,
              .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,},
    }};
    VkDescriptorPoolCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
        .maxSets = static_cast<uint32_t>(NUM_IMAGES_IN_SWAPCHAIN),
    };

    vkCreateDescriptorPool(vkDevice, &createInfo, nullptr, &descriptorPool);
}

void Engine::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uniformLayoutBinding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr,
    };
    VkDescriptorSetLayoutBinding samplerLayoutBinding{
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
    };

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uniformLayoutBinding,
            samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data(),
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
        .descriptorSetCount = NUM_IMAGES_IN_SWAPCHAIN,
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
        VkDescriptorImageInfo imageInfo{
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView = texture.imageView,
            .sampler = texture.sampler,
        };

        std::array<VkWriteDescriptorSet, 2> writeDescriptorSets = {{
               {
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
               },
               {
                   .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                   .pNext = nullptr,
                   .dstSet = descriptorSets[i],
                   .dstBinding = 1,
                   .dstArrayElement = 0,
                   .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                   .descriptorCount = 1,
                   .pBufferInfo = nullptr,
                   .pImageInfo = &imageInfo,
                   .pTexelBufferView = nullptr,
               },
              }};
        vkUpdateDescriptorSets(vkDevice, static_cast<uint32_t>(writeDescriptorSets.size()),
                writeDescriptorSets.data(), 0, nullptr);
    }
}

VkCommandBuffer Engine::beginSingleTimeCommands() const {
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

void Engine::endSingleTimeCommands(VkCommandBuffer commandBuffer) const {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
            .pNext = nullptr,
            .pWaitSemaphores = nullptr,
            .waitSemaphoreCount = 0,
            .pSignalSemaphores = nullptr,
            .signalSemaphoreCount = 0,
            .pWaitDstStageMask = nullptr,
    };
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);
    vkFreeCommandBuffers(vkDevice, commandPool, 1, &commandBuffer);
}

void Engine::loadResources() {
    geometry.loadFromFile("/storage/emulated/0/Documents/mydreamland/geometry/vertices.dat");
    texture.loadFromFile(this, "/storage/emulated/0/Documents/mydreamland/texture/texture.jpg");
}