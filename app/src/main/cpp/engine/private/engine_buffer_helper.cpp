//
// Created by andys on 6/3/2019.
//

#include <engine.h>

uint32_t Engine::findOptimalMemoryTypeIndexSupportSpecifiedPropertyFlags(
        uint32_t targetMemoryTypeBits,
        VkMemoryPropertyFlags targetMemoryPropertyFlags) const {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memoryProperties);
    for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if(targetMemoryTypeBits & (1 << i) &&
        (memoryProperties.memoryTypes[i].propertyFlags & targetMemoryPropertyFlags) ==
        targetMemoryPropertyFlags) {
            return i;
        }
    }
    throw std::runtime_error(
            "Engine::findOptimalMemoryTypeIndexSupportSpecifiedPropertyFlags failed!");
}

void Engine::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags propertyFlags, VkBuffer &buffer,
                  VkDeviceMemory &bufferMemory) const {
    VkBufferCreateInfo bufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
    };
    vkCreateBuffer(vkDevice, &bufferCreateInfo, nullptr, &buffer);

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(vkDevice, buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = findOptimalMemoryTypeIndexSupportSpecifiedPropertyFlags(
                    memoryRequirements.memoryTypeBits,
                    propertyFlags),
    };
    vkAllocateMemory(vkDevice, &allocateInfo, nullptr, &bufferMemory);

    vkBindBufferMemory(vkDevice, buffer, bufferMemory, 0);
}

void Engine::copyBuffer(VkBuffer srcBuffer, VkBuffer destBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy bufferCopy{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size,
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, destBuffer, 1, &bufferCopy);

    endSingleTimeCommands(commandBuffer);
}
