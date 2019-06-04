//
// Created by andys on 6/3/2019.
//

#include <engine.h>

uint32_t Engine::findOptimalMemoryTypeIndexSupportSpecifiedPropertyFlags(
        uint32_t targetMemoryTypeBits,
        VkMemoryPropertyFlags targetMemoryPropertyFlags) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memoryProperties);
    for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if(targetMemoryTypeBits & (1 << i) &&
        (memoryProperties.memoryTypes[i].propertyFlags & targetMemoryPropertyFlags) ==
        targetMemoryPropertyFlags) {
            return i;
        }
    }
    throw "Engine::findOptimalMemoryTypeIndexSupportSpecifiedPropertyFlags failed!";
}

void Engine::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags propertyFlags, VkBuffer &buffer,
                  VkDeviceMemory &bufferMemory) {
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

    VkBufferCopy bufferCopy{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size,
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, destBuffer, 1, &bufferCopy);

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
