//
// Created by andys on 6/4/2019.
//

#define STB_IMAGE_IMPLEMENTATION
#include <engine.h>

void Engine::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format,
                         VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags propertyFlags, VkImage &image,
                         VkDeviceMemory &imageMemory) const {
    VkImageCreateInfo imageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
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
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .flags = 0,
            .pNext = nullptr,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
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

void Texture::loadFromFile(const Engine *engine, const char *fileName) {
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

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    engine->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(engine->vkDevice, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(engine->vkDevice, stagingBufferMemory);

    stbi_image_free(pixels);

    engine->createImage(texWidth, texHeight, 1, VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);

    VkCommandBuffer commandBuffer = engine->beginSingleTimeCommands();

    engine->transitionImageLayout(commandBuffer, image, VK_IMAGE_ASPECT_COLOR_BIT,
                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          1);
    engine->copyBufferToImage(commandBuffer, stagingBuffer, image, static_cast<uint32_t>(texWidth),
                      static_cast<uint32_t>(texHeight));
    engine->transitionImageLayout(commandBuffer, image, VK_IMAGE_ASPECT_COLOR_BIT,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

    engine->endSingleTimeCommands(commandBuffer);

    vkDestroyBuffer(engine->vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(engine->vkDevice, stagingBufferMemory, nullptr);

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
                                   VkImageLayout newLayout, uint32_t mipLevels) const {
    VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .pNext = nullptr,
        .image = image,
        .subresourceRange.aspectMask = aspectFlags,
        .subresourceRange.layerCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.levelCount = mipLevels,
        .subresourceRange.baseMipLevel = 0,
        .srcAccessMask = 0,
        .dstAccessMask = 0,
    };

    VkPipelineStageFlags srcStage, dstStage;
    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
       newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0,
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }  else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        throw std::runtime_error("Engine::transitionImageLayout: unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void Texture::destroy(const Engine *engine) {
    vkDestroySampler(engine->vkDevice, sampler, nullptr);
    vkDestroyImageView(engine->vkDevice, imageView, nullptr);
    vkDestroyImage(engine->vkDevice, image, nullptr);
    vkFreeMemory(engine->vkDevice, imageMemory, nullptr);
}

void Engine::copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image,
        uint32_t width, uint32_t height) const {
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
        uint32_t mipLevels) const {
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

