//
// Created by andys on 6/4/2019.
//

#define STB_IMAGE_IMPLEMENTATION
#include <engine.h>

void Engine::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format,
                         VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags propertyFlags, VkImage &image,
                         VkDeviceMemory &imageMemory) {
    VkImageCreateInfo imageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .extent.width = width,
            .extent.height = height,
            .extent.depth = 1,
            .mipLevels = mipLevels,
            .arrayLayers = 1,
            .format = format,
            .tiling = tiling,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .samples = VK_SAMPLE_COUNT_1_BIT,
    };
    vkCreateImage(vkDevice, &imageCreateInfo, nullptr, &image);

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(vkDevice, image, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = findOptimalMemoryTypeIndexSupportSpecifiedPropertyFlags(
                    memoryRequirements.memoryTypeBits, propertyFlags),
    };
    vkAllocateMemory(vkDevice, &allocateInfo, nullptr, &imageMemory);

    vkBindImageMemory(vkDevice, image, imageMemory, 0);
}

void Texture::initFromFile(Engine *engine, VkCommandBuffer commandBuffer, VkBuffer &stagingBuffer,
                           VkDeviceMemory &stagingBufferMemory, const char *fileName) {
    FILE *f = fopen(fileName, "r");
    fseek(f, 0, SEEK_END);
    auto nBytes = static_cast<uint32_t>(ftell(f));
    fseek(f, 0, SEEK_SET);
    std::vector<uint8_t> content(nBytes);
    fread(content.data(), 1, nBytes, f);
    fclose(f);

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load_from_memory(content.data(), content.size(),
                                            &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = static_cast<uint32_t>(texWidth * texHeight * 4);

    if (!pixels) {
        throw std::runtime_error("Engine::createTextureImage failed!");
    }

    engine->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(engine->vkDevice, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(engine->vkDevice, stagingBufferMemory);

    stbi_image_free(pixels);

    engine->createImage(texWidth, texHeight, 1, VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                image, imageMemory);

    /* GPU-side commands */

    /* vkQueueSubmit takes care of necessary memory domain & visibility operations */
    engine->transitionImageLayout(commandBuffer, image, VK_IMAGE_ASPECT_COLOR_BIT,
                                  VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  1, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0);
    engine->copyBufferToImage(commandBuffer, stagingBuffer, image, static_cast<uint32_t>(texWidth),
                      static_cast<uint32_t>(texHeight));
    engine->transitionImageLayout(commandBuffer, image, VK_IMAGE_ASPECT_COLOR_BIT,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1,
                                  VK_PIPELINE_STAGE_TRANSFER_BIT,
                                  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                  VK_ACCESS_TRANSFER_WRITE_BIT, 0);
    /* vkQueueSubmit fence signal operation will make all writes available */

    imageView = engine->createImageView(image, VK_FORMAT_R8G8B8A8_UNORM,
                                       VK_IMAGE_ASPECT_COLOR_BIT, 1);

    VkSamplerCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = 16,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .mipLodBias = 0.0f,
            .minLod = 0.0f,
            .maxLod = 0.0f,
    };
    vkCreateSampler(engine->vkDevice, &createInfo, nullptr, &sampler);
}

void Engine::transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image,
                                   VkImageAspectFlags aspectFlags, VkImageLayout oldLayout,
                                   VkImageLayout newLayout, uint32_t mipLevels,
                                   VkPipelineStageFlags srcStageMask,
                                   VkPipelineStageFlags dstStageMask, VkAccessFlags srcAccessMask,
                                   VkAccessFlags dstAccessMask) {
    VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .image = image,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .subresourceRange.aspectMask = aspectFlags,
        .subresourceRange.layerCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.levelCount = mipLevels,
        .subresourceRange.baseMipLevel = 0,
        .srcAccessMask = srcAccessMask,
        .dstAccessMask = dstAccessMask,
    };

    vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr,
            1, &barrier);
}

void Texture::destroy(Engine *engine) {
    vkDestroySampler(engine->vkDevice, sampler, nullptr);
    vkDestroyImageView(engine->vkDevice, imageView, nullptr);
    vkDestroyImage(engine->vkDevice, image, nullptr);
    vkFreeMemory(engine->vkDevice, imageMemory, nullptr);
}

void Engine::copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image,
        uint32_t width, uint32_t height) {
    VkBufferImageCopy bufferImageCopy{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .imageSubresource.mipLevel = 0,
        .imageSubresource.baseArrayLayer = 0,
        .imageSubresource.layerCount = 1,
        .imageOffset = {0, 0, 0},
        .imageExtent = {width, height, 1},
    };
    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &bufferImageCopy);
}

VkImageView Engine::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
        uint32_t mipLevels) {
    VkImageViewCreateInfo imgViewCreateInfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = aspectFlags,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = mipLevels,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1,
    };
    VkImageView imageView;
    vkCreateImageView(vkDevice, &imgViewCreateInfo, nullptr,
                      &imageView);
    return imageView;
}

