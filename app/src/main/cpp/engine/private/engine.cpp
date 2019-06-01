//
// Created by andys on 6/1/2019.
//

#include "engine.h"

Engine gEngine;

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(
        VkDebugReportFlagsEXT msgFlags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t srcObject, size_t location,
        int32_t msgCode, const char *pLayerPrefix,
        const char *pMsg, void * pUserData )
{
    if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        __android_log_print(ANDROID_LOG_ERROR,
                            "main",
                            "ERROR: [%s] Code %i : %s",
                            pLayerPrefix, msgCode, pMsg);
    } else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        __android_log_print(ANDROID_LOG_WARN,
                            "main",
                            "WARNING: [%s] Code %i : %s",
                            pLayerPrefix, msgCode, pMsg);
    } else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        __android_log_print(ANDROID_LOG_WARN,
                            "main",
                            "PERFORMANCE WARNING: [%s] Code %i : %s",
                            pLayerPrefix, msgCode, pMsg);
    } else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
        __android_log_print(ANDROID_LOG_INFO,
                            "main", "INFO: [%s] Code %i : %s",
                            pLayerPrefix, msgCode, pMsg);
    } else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
        __android_log_print(ANDROID_LOG_VERBOSE,
                            "main", "DEBUG: [%s] Code %i : %s",
                            pLayerPrefix, msgCode, pMsg);
    }

    // Returning false tells the layer not to stop when the event occurs, so
    // they see the same behavior with and without validation layers enabled.
    return VK_FALSE;
}

int Engine::initDisplay() {
    // print available instance extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    __android_log_print(ANDROID_LOG_INFO, "main", "Vulkan Available Extensions:");
    for(const auto &extension: extensions) {
        __android_log_print(ANDROID_LOG_INFO, "main", "\tExtension Name: %s\t\tVersion: %d",
                extension.extensionName,extension.specVersion);
    }

    // print available layers
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layers(layerCount);
    std::vector<const char *> layerNames;
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
    __android_log_print(ANDROID_LOG_INFO, "main", "Vulkan Available Layers:");
    for(const auto &layer: layers) {
        __android_log_print(ANDROID_LOG_INFO, "main",
                "\tLayer Name: %s\t\tSpec Version: %d\t\tImpl Version: %d\n\t\tDesc: %s",
                layer.layerName, layer.specVersion, layer.implementationVersion,
                layer.description);
        layerNames.push_back(static_cast<const char *>(layer.layerName));
    }

    VkApplicationInfo appInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .apiVersion = VK_MAKE_VERSION(1, 0, 66),
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .pApplicationName = "mydreamland",
            .pEngineName = "mydreamland_engine",
    };

    // prepare necessary extensions: Vulkan on Android need these to function
    std::vector<const char *> instanceExt, deviceExt;
    instanceExt.push_back("VK_KHR_surface");
    instanceExt.push_back("VK_KHR_android_surface");
    if(DEBUG_ON && layerCount > 0) {
        instanceExt.push_back("VK_EXT_debug_report");
    }
    deviceExt.push_back("VK_KHR_swapchain");

    // create the Vulkan instance
    VkInstanceCreateInfo instanceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .pApplicationInfo = &appInfo,
            .enabledExtensionCount = static_cast<uint32_t>(instanceExt.size()),
            .ppEnabledExtensionNames = instanceExt.data(),
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
    };
    if(DEBUG_ON && layerCount > 0) {
        instanceCreateInfo.enabledLayerCount = layerCount;
        instanceCreateInfo.ppEnabledLayerNames = layerNames.data();
    }
    vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance);

    // create Vulkan Debug Report Callback
    if(DEBUG_ON &&layerCount > 0) {
        VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
                .pNext = nullptr,
                .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                         VK_DEBUG_REPORT_WARNING_BIT_EXT |
                         VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                         VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                         VK_DEBUG_REPORT_DEBUG_BIT_EXT,
                .pfnCallback = DebugReportCallback,
                .pUserData = nullptr
        };
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackExt;
        vkCreateDebugReportCallbackExt = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
                vkInstance, "vkCreateDebugReportCallbackEXT");
        vkCreateDebugReportCallbackExt(vkInstance, &debugReportCallbackCreateInfo, nullptr,
                                       &vkDebugReportCallbackExt);
    }

    // if we create a surface, we need the surface extension
    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfoKhr{
            .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .window = app->window};
    vkCreateAndroidSurfaceKHR(vkInstance, &surfaceCreateInfoKhr, nullptr,
                              &vkSurface);

    // Find one GPU to use:
    // on Android, every GPU device is equal -- supporting graphics/compute/present
    // for this sample, we use the very first GPU device found on the system
    uint32_t gpuCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &gpuCount, nullptr);
    assert(gpuCount > 0);
    VkPhysicalDevice tmpGpus[gpuCount];
    vkEnumeratePhysicalDevices(vkInstance, &gpuCount, tmpGpus);
    vkGpu = tmpGpus[0];  // Pick up the first GPU Device

    // check for Vulkan info on this GPU device
    VkPhysicalDeviceProperties gpuProperties;
    vkGetPhysicalDeviceProperties(vkGpu, &gpuProperties);
    __android_log_print(ANDROID_LOG_INFO, "main", "Vulkan Physical Device Name: %s",
            gpuProperties.deviceName);
    __android_log_print(ANDROID_LOG_INFO, "main", "Vulkan Physical Device Id: %d",
            gpuProperties.deviceID);
    __android_log_print(ANDROID_LOG_INFO, "main", "Vulkan Physical Device Type: %d",
            gpuProperties.deviceType);
    __android_log_print(ANDROID_LOG_INFO, "main", "Vulkan Physical Device Vendor Id: %d",
            gpuProperties.vendorID);
    __android_log_print(ANDROID_LOG_INFO, "main",
            "Vulkan Physical Device Info: apiVersion: %x \n\t driverVersion: %x",
         gpuProperties.apiVersion, gpuProperties.driverVersion);
    __android_log_print(ANDROID_LOG_INFO, "main", "API Version Supported: %d.%d.%d",
         VK_VERSION_MAJOR(gpuProperties.apiVersion),
         VK_VERSION_MINOR(gpuProperties.apiVersion),
         VK_VERSION_PATCH(gpuProperties.apiVersion));

    VkPhysicalDeviceFeatures gpuFeatures;
    vkGetPhysicalDeviceFeatures(vkGpu, &gpuFeatures);
    __android_log_print(ANDROID_LOG_INFO, "main",
            "Vulkan Physical Device Support Tessellation Shader: %d", gpuFeatures.tessellationShader);
    __android_log_print(ANDROID_LOG_INFO, "main",
            "Vulkan Physical Device Support Geometry Shader: %d", gpuFeatures.geometryShader);
    __android_log_print(ANDROID_LOG_INFO, "main",
            "Vulkan Physical Device Support Index32: %d", gpuFeatures.fullDrawIndexUint32);

    uint32_t deviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(vkGpu, nullptr, &deviceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> deviceExtension(deviceExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &deviceExtensionCount, deviceExtension.data());
    __android_log_print(ANDROID_LOG_INFO, "main", "Vulkan Physical Device Available Extensions:\n");
    for(const auto &extension: deviceExtension) {
        __android_log_print(ANDROID_LOG_INFO, "main",
                "\tExtension Name: %s\t\tVersion: %d\n", extension.extensionName,
             extension.specVersion);
    }

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkGpu, vkSurface,
                                              &surfaceCapabilities);

    __android_log_print(ANDROID_LOG_INFO, "main", "Vulkan Surface Capabilities:\n");
    __android_log_print(ANDROID_LOG_INFO, "main",
            "\timage count: %u - %u\n", surfaceCapabilities.minImageCount,
         surfaceCapabilities.maxImageCount);
    __android_log_print(ANDROID_LOG_INFO, "main",
            "\tarray layers: %u\n", surfaceCapabilities.maxImageArrayLayers);
    __android_log_print(ANDROID_LOG_INFO, "main",
            "\timage size (now): %dx%d\n", surfaceCapabilities.currentExtent.width,
         surfaceCapabilities.currentExtent.height);
    __android_log_print(ANDROID_LOG_INFO, "main", "\timage size (extent): %dx%d - %dx%d\n",
         surfaceCapabilities.minImageExtent.width,
         surfaceCapabilities.minImageExtent.height,
         surfaceCapabilities.maxImageExtent.width,
         surfaceCapabilities.maxImageExtent.height);
    __android_log_print(ANDROID_LOG_INFO, "main",
            "\tusage: %x\n", surfaceCapabilities.supportedUsageFlags);
    __android_log_print(ANDROID_LOG_INFO, "main",
            "\tcurrent transform: %u\n", surfaceCapabilities.currentTransform);
    __android_log_print(ANDROID_LOG_INFO, "main",
            "\tallowed transforms: %x\n", surfaceCapabilities.supportedTransforms);
    __android_log_print(ANDROID_LOG_INFO, "main",
            "\tcomposite alpha flags: %u\n", surfaceCapabilities.supportedCompositeAlpha);

    // Find the family index of a graphics queue family
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(vkGpu, &queueFamilyCount, nullptr);
    assert(queueFamilyCount);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkGpu, &queueFamilyCount,
                                             queueFamilyProperties.data());

    uint32_t queueFamilyIndex;
    for (queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; queueFamilyIndex++) {
        if (queueFamilyProperties[queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            break;
        }
    }
    assert(queueFamilyIndex < queueFamilyCount);

    VkBool32 b_presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(vkGpu, queueFamilyIndex, vkSurface,
                                         &b_presentSupport);
    assert(b_presentSupport);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkGpu, vkSurface, &formatCount, nullptr);
    assert(formatCount > 0);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkGpu, vkSurface, &formatCount,
                                         formats.data());
    VkSurfaceFormatKHR surfaceFormatChosen = {VK_FORMAT_R8G8B8A8_UNORM,
                                              VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    bool b_formatSupport = false;
    __android_log_print(ANDROID_LOG_INFO, "main",
            "Vulkan Physical Device Supported Formats:\n");
    for(const auto &format: formats) {
        if(format.format == surfaceFormatChosen.format && format.colorSpace ==
                                                          surfaceFormatChosen.colorSpace) {
            b_formatSupport = true;
        }
        if(format.format == VK_FORMAT_UNDEFINED) {
            b_formatSupport = true;
        }
        __android_log_print(ANDROID_LOG_INFO, "main",
                "\tFormat: %d\tColor Space: %d\n", format.format, format.colorSpace);
    }
    assert(b_formatSupport);
    swapChainImageFormat = surfaceFormatChosen.format;

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkGpu, vkSurface, &presentModeCount,
                                              nullptr);
    assert(presentModeCount > 0);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkGpu, vkSurface, &presentModeCount,
                                              presentModes.data());
    VkPresentModeKHR presentModeChosen = VK_PRESENT_MODE_FIFO_KHR;
    bool b_presentModeSupport = false;
    __android_log_print(ANDROID_LOG_INFO, "main",
            "Vulkan Physical Device Supported Present Modes:\n");
    for(const auto &pm: presentModes) {
        if(pm == presentModeChosen) {
            b_presentModeSupport = true;
        }
        __android_log_print(ANDROID_LOG_INFO, "main", "\tPresent Mode: %d\n", pm);
    }
    assert(b_presentModeSupport);

    // Create a logical device from GPU we picked
    float priorities[] = {
            1.0f,
    };
    VkDeviceQueueCreateInfo queueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCount = 1,
            .queueFamilyIndex = queueFamilyIndex,
            .pQueuePriorities = priorities,
    };
    VkDeviceCreateInfo deviceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueCreateInfo,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<uint32_t>(deviceExt.size()),
            .ppEnabledExtensionNames = deviceExt.data(),
            .pEnabledFeatures = &gpuFeatures,
    };
    if(DEBUG_ON && layerCount > 0) {
        deviceCreateInfo.enabledLayerCount = layerCount;
        deviceCreateInfo.ppEnabledLayerNames = layerNames.data();
    }
    vkCreateDevice(vkGpu, &deviceCreateInfo, nullptr, &vkDevice);

    // get queue from logical device
    vkGetDeviceQueue(vkDevice, queueFamilyIndex, 0, &vkQueue);

    // create swap chain
    assert(surfaceCapabilities.minImageCount > 1);
    uint32_t imageCount = surfaceCapabilities.minImageCount;
    if(imageCount + 1 <= surfaceCapabilities.maxImageCount) {
        imageCount += 1;
    }
    VkSwapchainCreateInfoKHR swapchainCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = vkSurface,
            .minImageCount = imageCount,
            .imageFormat = surfaceFormatChosen.format,
            .imageColorSpace = surfaceFormatChosen.colorSpace,
            .imageExtent = surfaceCapabilities.currentExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
            .presentMode = presentModeChosen,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE,
    };
    vkCreateSwapchainKHR(vkDevice, &swapchainCreateInfo, nullptr, &vkSwapchain);

    // retrieve swap chain images
    vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &imageCount,
                            swapChainImages.data());

    // create swap chain image views
    swapChainImageViews.resize(swapChainImages.size());
    for(int i = 0; i < swapChainImageViews.size(); i++) {
        VkImageViewCreateInfo imgViewCreateInfo {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = swapChainImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = swapChainImageFormat,
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

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr,
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport viewport{
            .x = 0,
            .y = 0,
            .width = static_cast<float>(surfaceCapabilities.currentExtent.width),
            .height = static_cast<float>(surfaceCapabilities.currentExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
    };
    VkRect2D scissor{
            .offset = {0, 0},
            .extent = surfaceCapabilities.currentExtent,
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
            .depthClampEnable = VK_TRUE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .lineWidth = 1.0f,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
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

    VkPipelineColorBlendAttachmentState colorBlendAttachment{
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
            .pAttachments = &colorBlendAttachment,
            .blendConstants[0] = 0.0f,
            .blendConstants[1] = 0.0f,
            .blendConstants[2] = 0.0f,
            .blendConstants[3] = 0.0f,
    };

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
    };
    vkCreatePipelineLayout(vkDevice, &pipelineLayoutCreateInfo, nullptr,
                           &pipelineLayout);

    VkAttachmentDescription colorAttachment{
            .format = swapChainImageFormat,
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
    VkSubpassDependency subpassDependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,

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
            .pAttachments = &colorAttachment,
            .subpassCount = 1,
            .pSubpasses = &subpassDescription,
            .dependencyCount = 1,
            .pDependencies = &subpassDependency,
    };
    vkCreateRenderPass(vkDevice, &renderPassCreateInfo, nullptr, &renderPass);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssemblyInfo,
            .pRasterizationState = &rasterizationStateCreateInfo,
            .pMultisampleState = &multisampleStateCreateInfo,
            .pDepthStencilState = nullptr,
            .pViewportState = &viewportStateCreateInfo,
            .pColorBlendState = &colorBlendStateCreateInfo,
            .pDynamicState = nullptr,
            .layout = pipelineLayout,
            .renderPass = renderPass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1,
    };
    vkCreateGraphicsPipelines(vkDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr,
                              &graphicsPipeline);

    vkDestroyShaderModule(vkDevice, vertShaderModule, nullptr);
    vkDestroyShaderModule(vkDevice, fragShaderModule, nullptr);

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
                .width = surfaceCapabilities.currentExtent.width,
                .height = surfaceCapabilities.currentExtent.height,
                .layers = 1
        };
        vkCreateFramebuffer(vkDevice, &framebufferCreateInfo, nullptr,
                            &swapChainFrameBuffers[i]);
    }

    // create command pool
    VkCommandPoolCreateInfo commandPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = queueFamilyIndex,
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
                .renderArea.extent = surfaceCapabilities.currentExtent,
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
    vkQueueSubmit(vkQueue, 1, &submitInfo, inFlightFences[currentFrame]);

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
    vkQueuePresentKHR(vkQueue, &presentInfo);

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
    vkDestroyPipelineLayout(vkDevice, pipelineLayout, nullptr);

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
