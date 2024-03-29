//
// Created by andys on 6/1/2019.
//
#include <engine.h>

VKAPI_ATTR VkBool32 VKAPI_CALL Engine::debugReportCallback(
        VkDebugReportFlagsEXT msgFlags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t srcObject, size_t location,
        int32_t msgCode, const char *pLayerPrefix,
        const char *pMsg, void *pUserData)
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

void Engine::logAvailableInstanceExtensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    __android_log_print(ANDROID_LOG_INFO, "main", "Vulkan Available Instance Extensions:");
    for(const auto &extension: extensions) {
        __android_log_print(ANDROID_LOG_INFO, "main", "\tExtension Name: %s\t\tVersion: %d",
                            extension.extensionName,extension.specVersion);
    }
}

#ifdef DEBUG
void Engine::updateAvailableValidationLayerNames() {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    if(layerCount < 1) {
        throw std::runtime_error("validation layers not found!");
    }

    validationLayerProperties->resize(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, validationLayerProperties->data());
    __android_log_print(ANDROID_LOG_INFO, "main", "Vulkan Available Validation Layers:");
    validationLayerNames->resize(validationLayerProperties->size());
    for(size_t i = 0; i < validationLayerProperties->size(); i++) {
        __android_log_print(ANDROID_LOG_INFO, "main",
                            "\tLayer Name: %s\t\tSpec Version: %d\t\tImpl Version: %d\n\tDesc: %s",
                            (*validationLayerProperties)[i].layerName,
                            (*validationLayerProperties)[i].specVersion,
                            (*validationLayerProperties)[i].implementationVersion,
                            (*validationLayerProperties)[i].description);
        (*validationLayerNames)[i] =
                static_cast<const char *>((*validationLayerProperties)[i].layerName);
    }
}

void Engine::createVKDebugReportCallback() {
    VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                     VK_DEBUG_REPORT_WARNING_BIT_EXT |
                     VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                     VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                     VK_DEBUG_REPORT_DEBUG_BIT_EXT,
            .pfnCallback = debugReportCallback,
            .pUserData = nullptr
    };
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackExt;
    vkCreateDebugReportCallbackExt = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
            vkInstance, "vkCreateDebugReportCallbackEXT");
    vkCreateDebugReportCallbackExt(vkInstance, &debugReportCallbackCreateInfo, nullptr,
                                   &vkDebugReportCallbackExt);
}
#endif

void Engine::logPhysicalDeviceProperties() {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(vkPhysicalDevice, &physicalDeviceProperties);
    __android_log_print(ANDROID_LOG_INFO, "main", "Vulkan Selected Physical Device Properties:");
    __android_log_print(ANDROID_LOG_INFO, "main", "\tName: %s",
                        physicalDeviceProperties.deviceName);
    __android_log_print(ANDROID_LOG_INFO, "main", "\tId: %d",
                        physicalDeviceProperties.deviceID);
    __android_log_print(ANDROID_LOG_INFO, "main", "\tType: %d",
                        physicalDeviceProperties.deviceType);
    __android_log_print(ANDROID_LOG_INFO, "main", "\tVendor Id: %d",
                        physicalDeviceProperties.vendorID);
    __android_log_print(ANDROID_LOG_INFO, "main",
                        "\tInfo: apiVersion: %x \t\t driverVersion: %x",
                        physicalDeviceProperties.apiVersion, physicalDeviceProperties.driverVersion);
    __android_log_print(ANDROID_LOG_INFO, "main", "API Version Supported: %d.%d.%d",
                        VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion),
                        VK_VERSION_MINOR(physicalDeviceProperties.apiVersion),
                        VK_VERSION_PATCH(physicalDeviceProperties.apiVersion));
}

void Engine::logPhysicalDeviceAvailableExtensions() {
    uint32_t deviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &deviceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> deviceExtension(deviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &deviceExtensionCount, deviceExtension.data());
    __android_log_print(ANDROID_LOG_INFO, "main",
            "Vulkan Selected Physical Device Available Extensions:");
    for(const auto &extension: deviceExtension) {
        __android_log_print(ANDROID_LOG_INFO, "main",
                            "\tExtension Name: %s\t\tVersion: %d", extension.extensionName,
                            extension.specVersion);
    }
}

void Engine::checkPhysicalDeviceGraphicsQueueSurfaceSupport() {
    VkBool32 b_presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, physicalDeviceGraphicsQueueFamilyIndex,
                                         vkSurface, &b_presentSupport);
    assert(b_presentSupport);
}


