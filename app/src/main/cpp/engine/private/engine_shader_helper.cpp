//
// Created by andys on 5/31/2019.
//

#include "engine.h"

std::vector<char> Engine::readFile(const char *fileName) {
    AAsset* shaderFile = AAssetManager_open(activity->assetManager,
                                            fileName, AASSET_MODE_BUFFER);
    size_t shaderFileLen = AAsset_getLength(shaderFile);
    std::vector<char> content(shaderFileLen);
    AAsset_read(shaderFile, content.data(), shaderFileLen);
    AAsset_close(shaderFile);
    return content;
}

VkShaderModule Engine::createShaderModule(const std::vector<char> &code) {
    VkShaderModuleCreateInfo vertShaderModuleCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size(),
            .pCode = reinterpret_cast<const uint32_t *>(code.data()),
    };
    VkShaderModule shaderModule;
    vkCreateShaderModule(vkDevice, &vertShaderModuleCreateInfo, nullptr, &shaderModule);
    return shaderModule;
}

void Engine::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding binding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr,
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = 1,
            .pBindings = &binding,
    };
    vkCreateDescriptorSetLayout(vkDevice, &descriptorSetLayoutCreateInfo, nullptr,
                                &descriptorSetLayout);
}

