//
// Created by andys on 6/3/2019.
//

#include <engine.h>

uint32_t Engine::findMemoryTypeIndex(uint32_t targetMemoryTypeBits,
                                     VkMemoryPropertyFlags targetMemoryPropertyFlags) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memoryProperties);
    for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if(targetMemoryTypeBits & (1 << i) &&
        (memoryProperties.memoryTypes[i].propertyFlags & targetMemoryPropertyFlags) == targetMemoryPropertyFlags) {
            return i;
        }
    }
    throw "Engine::findMemoryTypeIndex failed!";
}