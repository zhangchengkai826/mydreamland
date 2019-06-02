//
// Created by andys on 6/2/2019.
//

#include <engine.h>

void Engine::checkAvailableValidationLayers() {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
    __android_log_print(ANDROID_LOG_INFO, "main", "Vulkan Available Validation Layers:");
    for(const auto &layer: layers) {
        __android_log_print(ANDROID_LOG_INFO, "main",
                            "\tLayer Name: %s\t\tSpec Version: %d\t\tImpl Version: %d\n\t\tDesc: %s",
                            layer.layerName, layer.specVersion, layer.implementationVersion,
                            layer.description);
        validationLayerNames.push_back(static_cast<const char *>(layer.layerName));
    }
}

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
        instanceCreateInfo.enabledLayerCount = validationLayerNames.size();
        instanceCreateInfo.ppEnabledLayerNames = validationLayerNames.data();
    }
    vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance);
}
