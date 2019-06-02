//
// Created by andys on 5/31/2019.
//

#include "engine.h"

std::vector<char> Engine::readFile(const char *fileName) {
    AAsset* shaderFile = AAssetManager_open(app->activity->assetManager,
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

void Engine::createGraphicsPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
    };
    vkCreatePipelineLayout(vkDevice, &pipelineLayoutCreateInfo, nullptr,
            &graphicsPipelineLayout);
}

