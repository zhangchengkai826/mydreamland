//
// Created by andys on 5/31/2019.
//

#include "engine_shader_helper.h"

std::vector<char> readFile(struct engine *engine, const char *fileName) {
    AAsset* shaderFile = AAssetManager_open(engine->app->activity->assetManager,
                                            fileName, AASSET_MODE_BUFFER);
    size_t shaderFileLen = AAsset_getLength(shaderFile);
    std::vector<char> code(shaderFileLen);
    AAsset_read(shaderFile, code.data(), shaderFileLen);
    AAsset_close(shaderFile);
    return code;
}

VkShaderModule createShaderModule(struct engine *engine, const std::vector<char> &code) {
    VkShaderModuleCreateInfo vertShaderModuleCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size(),
            .pCode = reinterpret_cast<const uint32_t *>(code.data()),
    };
    VkShaderModule shaderModule;
    vkCreateShaderModule(engine->vkDevice, &vertShaderModuleCreateInfo, nullptr, &shaderModule);
    return shaderModule;
}


