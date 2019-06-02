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
            .window = app->window};
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

void Engine::updatePhysicalDeviceSurfaceCapabilities() {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, vkSurface,
                                              &physicalDeviceSurfaceCapabilities);

    __android_log_print(ANDROID_LOG_INFO, "main", "Vulkan physicalDeviceSurface Capabilities:\n");
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "\timage count: %u - %u\n", physicalDeviceSurfaceCapabilities.minImageCount,
                        physicalDeviceSurfaceCapabilities.maxImageCount);
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "\tarray layers: %u\n",
                        physicalDeviceSurfaceCapabilities.maxImageArrayLayers);
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "\timage size (now): %dx%d\n",
                        physicalDeviceSurfaceCapabilities.currentExtent.width,
                        physicalDeviceSurfaceCapabilities.currentExtent.height);
    __android_log_print(ANDROID_LOG_INFO, "main", "\timage size (extent): %dx%d - %dx%d\n",
                        physicalDeviceSurfaceCapabilities.minImageExtent.width,
                        physicalDeviceSurfaceCapabilities.minImageExtent.height,
                        physicalDeviceSurfaceCapabilities.maxImageExtent.width,
                        physicalDeviceSurfaceCapabilities.maxImageExtent.height);
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "\tusage: %x\n", physicalDeviceSurfaceCapabilities.supportedUsageFlags);
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "\tcurrent transform: %u\n",
                        physicalDeviceSurfaceCapabilities.currentTransform);
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "\tallowed transforms: %x\n",
                        physicalDeviceSurfaceCapabilities.supportedTransforms);
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "\tcomposite alpha flags: %u\n",
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