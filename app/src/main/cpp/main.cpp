//
// Created by andys on 5/10/2019.
//

#include <android_native_app_glue.h>
#include <vector>
#include <android/log.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "main", __VA_ARGS__)

static bool g_b_debug = true;

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

struct saved_state {
    int32_t x = 0;
    int32_t y = 0;
};

struct engine {
    struct android_app *app = nullptr;

    int animating = 0;
    struct saved_state state;

    VkInstance vkInstance = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT vkDebugReportCallbackExt = VK_NULL_HANDLE;
    VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
    VkPhysicalDevice vkGpu = VK_NULL_HANDLE;
    VkDevice vkDevice = VK_NULL_HANDLE;
    VkQueue vkQueue = VK_NULL_HANDLE;
    VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;
};

static int32_t engine_handle_input(struct android_app *app, AInputEvent *event) {
    auto engine = (struct engine *)app->userData;
    if(AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = 1;
        engine->state.x = (int32_t)AMotionEvent_getX(event, 0);
        engine->state.y = (int32_t)AMotionEvent_getY(event, 0);
        //LOGI("x: %d, y: %d", engine->state.x, engine->state.y);
        return 1;
    }
    return 0;
}

static int engine_init_display(struct engine *engine) {
    // print available instance extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    LOGI("Vulkan Available Extensions:\n");
    for(const auto &extension: extensions) {
        LOGI("\tExtension Name: %s\t\tVersion: %d\n", extension.extensionName,
                extension.specVersion);
    }

    // print available layers
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layers(layerCount);
    std::vector<const char *> layerNames;
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
    LOGI("Vulkan Available Layers:\n");
    for(const auto &layer: layers) {
        LOGI("\tLayer Name: %s\t\tSpec Version: %d\t\tImpl Version: %d\n\t\tDesc: %s\n",
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
    if(g_b_debug && layerCount > 0) {
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
    if(g_b_debug && layerCount > 0) {
        instanceCreateInfo.enabledLayerCount = layerCount;
        instanceCreateInfo.ppEnabledLayerNames = layerNames.data();
    }
    vkCreateInstance(&instanceCreateInfo, nullptr, &engine->vkInstance);

    // create Vulkan Debug Report Callback
    if(g_b_debug &&layerCount > 0) {
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
                engine->vkInstance, "vkCreateDebugReportCallbackEXT");
        vkCreateDebugReportCallbackExt(engine->vkInstance, &debugReportCallbackCreateInfo, nullptr,
                &engine->vkDebugReportCallbackExt);
    }

    // if we create a surface, we need the surface extension
    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfoKhr{
            .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .window = engine->app->window};
    vkCreateAndroidSurfaceKHR(engine->vkInstance, &surfaceCreateInfoKhr, nullptr,
                                      &engine->vkSurface);

    // Find one GPU to use:
    // on Android, every GPU device is equal -- supporting graphics/compute/present
    // for this sample, we use the very first GPU device found on the system
    uint32_t gpuCount = 0;
    vkEnumeratePhysicalDevices(engine->vkInstance, &gpuCount, nullptr);
    assert(gpuCount > 0);
    VkPhysicalDevice tmpGpus[gpuCount];
    vkEnumeratePhysicalDevices(engine->vkInstance, &gpuCount, tmpGpus);
    engine->vkGpu = tmpGpus[0];  // Pick up the first GPU Device

    // check for Vulkan info on this GPU device
    VkPhysicalDeviceProperties gpuProperties;
    vkGetPhysicalDeviceProperties(engine->vkGpu, &gpuProperties);
    LOGI("Vulkan Physical Device Name: %s", gpuProperties.deviceName);
    LOGI("Vulkan Physical Device Id: %d", gpuProperties.deviceID);
    // Vulkan Physical Device Type: VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
    LOGI("Vulkan Physical Device Type: %d", gpuProperties.deviceType);
    LOGI("Vulkan Physical Device Vendor Id: %d", gpuProperties.vendorID);
    LOGI("Vulkan Physical Device Info: apiVersion: %x \n\t driverVersion: %x",
         gpuProperties.apiVersion, gpuProperties.driverVersion);
    LOGI("API Version Supported: %d.%d.%d",
         VK_VERSION_MAJOR(gpuProperties.apiVersion),
         VK_VERSION_MINOR(gpuProperties.apiVersion),
         VK_VERSION_PATCH(gpuProperties.apiVersion));

    VkPhysicalDeviceFeatures gpuFeatures;
    vkGetPhysicalDeviceFeatures(engine->vkGpu, &gpuFeatures);
    LOGI("Vulkan Physical Device Support Tessellation Shader: %d", gpuFeatures.tessellationShader);
    LOGI("Vulkan Physical Device Support Geometry Shader: %d", gpuFeatures.geometryShader);
    LOGI("Vulkan Physical Device Support Index32: %d", gpuFeatures.fullDrawIndexUint32);

    uint32_t deviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(engine->vkGpu, nullptr, &deviceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> deviceExtension(deviceExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &deviceExtensionCount, deviceExtension.data());
    LOGI("Vulkan Physical Device Available Extensions:\n");
    for(const auto &extension: deviceExtension) {
        LOGI("\tExtension Name: %s\t\tVersion: %d\n", extension.extensionName,
             extension.specVersion);
    }

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(engine->vkGpu, engine->vkSurface,
                                              &surfaceCapabilities);

    LOGI("Vulkan Surface Capabilities:\n");
    LOGI("\timage count: %u - %u\n", surfaceCapabilities.minImageCount,
         surfaceCapabilities.maxImageCount);
    LOGI("\tarray layers: %u\n", surfaceCapabilities.maxImageArrayLayers);
    LOGI("\timage size (now): %dx%d\n", surfaceCapabilities.currentExtent.width,
         surfaceCapabilities.currentExtent.height);
    LOGI("\timage size (extent): %dx%d - %dx%d\n",
         surfaceCapabilities.minImageExtent.width,
         surfaceCapabilities.minImageExtent.height,
         surfaceCapabilities.maxImageExtent.width,
         surfaceCapabilities.maxImageExtent.height);
    LOGI("\tusage: %x\n", surfaceCapabilities.supportedUsageFlags);
    LOGI("\tcurrent transform: %u\n", surfaceCapabilities.currentTransform);
    LOGI("\tallowed transforms: %x\n", surfaceCapabilities.supportedTransforms);
    LOGI("\tcomposite alpha flags: %u\n", surfaceCapabilities.supportedCompositeAlpha);

    // Find the family index of a graphics queue family
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(engine->vkGpu, &queueFamilyCount, nullptr);
    assert(queueFamilyCount);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(engine->vkGpu, &queueFamilyCount,
                                             queueFamilyProperties.data());

    uint32_t queueFamilyIndex;
    for (queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; queueFamilyIndex++) {
        if (queueFamilyProperties[queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            break;
        }
    }
    assert(queueFamilyIndex < queueFamilyCount);

    VkBool32 b_presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(engine->vkGpu, queueFamilyIndex, engine->vkSurface,
            &b_presentSupport);
    assert(b_presentSupport);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(engine->vkGpu, engine->vkSurface, &formatCount, nullptr);
    assert(formatCount > 0);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(engine->vkGpu, engine->vkSurface, &formatCount,
                                         formats.data());
    VkSurfaceFormatKHR surfaceFormatChosen = {VK_FORMAT_R8G8B8A8_UNORM,
                                        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    bool b_formatSupport = false;
    LOGI("Vulkan Physical Device Supported Formats:\n");
    for(const auto &format: formats) {
        if(format.format == surfaceFormatChosen.format && format.colorSpace ==
                                                    surfaceFormatChosen.colorSpace) {
            b_formatSupport = true;
        }
        if(format.format == VK_FORMAT_UNDEFINED) {
            b_formatSupport = true;
        }
        LOGI("\tFormat: %d\tColor Space: %d\n", format.format, format.colorSpace);
    }
    assert(b_formatSupport);

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(engine->vkGpu, engine->vkSurface, &presentModeCount,
                                              nullptr);
    assert(presentModeCount > 0);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(engine->vkGpu, engine->vkSurface, &presentModeCount,
                                              presentModes.data());
    VkPresentModeKHR presentModeChosen = VK_PRESENT_MODE_FIFO_KHR;
    bool b_presentModeSupport = false;
    LOGI("Vulkan Physical Device Supported Present Modes:\n");
    for(const auto &pm: presentModes) {
        if(pm == presentModeChosen) {
            b_presentModeSupport = true;
        }
        LOGI("\tPresent Mode: %d\n", pm);
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
    if(g_b_debug && layerCount > 0) {
        deviceCreateInfo.enabledLayerCount = layerCount;
        deviceCreateInfo.ppEnabledLayerNames = layerNames.data();
    }
    vkCreateDevice(engine->vkGpu, &deviceCreateInfo, nullptr, &engine->vkDevice);

    // get queue from logical device
    vkGetDeviceQueue(engine->vkDevice, queueFamilyIndex, 0, &engine->vkQueue);

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
            .surface = engine->vkSurface,
            .minImageCount = imageCount,
            .imageFormat = surfaceFormatChosen.format,
            .imageColorSpace = surfaceFormatChosen.colorSpace,
            .imageExtent = surfaceCapabilities.currentExtent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = surfaceCapabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
            .presentMode = presentModeChosen,
            .clipped = VK_TRUE,
            .oldSwapchain = VK_NULL_HANDLE,
    };
    vkCreateSwapchainKHR(engine->vkDevice, &swapchainCreateInfo, nullptr, &engine->vkSwapchain);

    return 0;
}

static void engine_term_display(struct engine *engine) {
    vkDestroySwapchainKHR(engine->vkDevice, engine->vkSwapchain, nullptr);
    vkDestroyDevice(engine->vkDevice, nullptr);
    vkDestroySurfaceKHR(engine->vkInstance, engine->vkSurface, nullptr);

    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
    vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)
            vkGetInstanceProcAddr(engine->vkInstance, "vkDestroyDebugReportCallbackEXT");
    vkDestroyDebugReportCallbackEXT(engine->vkInstance, engine->vkDebugReportCallbackExt, nullptr);

    vkDestroyInstance(engine->vkInstance, nullptr);
}

static void engine_handle_cmd(struct android_app *app, int32_t cmd) {
    auto engine = (struct engine *)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)app->savedState) = engine->state;
            app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            engine_init_display(engine);
            //engine_draw_frame(engine);
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            break;
        case APP_CMD_LOST_FOCUS:
            engine->animating = 0;
            //engine_draw_frame(engine);
            break;
        default:
            break;
    }
}

void android_main(android_app *app)
{
    struct engine engine;

    app->userData = &engine;
    app->onAppCmd = engine_handle_cmd;
    app->onInputEvent = engine_handle_input;
    engine.app = app;

    if(app->savedState != nullptr) {
        engine.state = *(struct saved_state *)app->savedState;
    }

    while(true) {
        int events;
        struct android_poll_source *source;

        while(ALooper_pollAll(engine.animating ? 0 : -1, nullptr, &events,
                                       (void **)&source) >= 0) {
            if(source != nullptr) {
                source->process(app, source);
            }

            if(app->destroyRequested != 0) {
                return;
            }
        }

        if(engine.animating) {
            //engine_draw_frame(&engine);
            engine.animating = 0;
        }
    }
}