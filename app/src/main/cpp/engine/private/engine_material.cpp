//
// Created by andys on 6/6/2019.
//

#include <engine.h>

void Material::init(const Engine *engine, const Texture *texture) {
    createDescriptorSetLayout(engine);
    createDescriptorPool(engine);
    createDescriptorSets(engine, texture);
    createGraphicsPipelineLayout(engine);
    createGraphicsPipeline(engine);
}

void Material::destroy(Engine *engine) {
    vkDestroyPipeline(engine->vkDevice, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(engine->vkDevice, graphicsPipelineLayout, nullptr);
    vkDestroyDescriptorPool(engine->vkDevice, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(engine->vkDevice, descriptorSetLayout, nullptr);
}

void Material::createDescriptorSetLayout(const Engine *engine) {
    VkDescriptorSetLayoutBinding uniformLayoutBinding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr,
    };
    VkDescriptorSetLayoutBinding samplerLayoutBinding{
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
    };

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uniformLayoutBinding,
                                                            samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data(),
    };
    vkCreateDescriptorSetLayout(engine->vkDevice, &descriptorSetLayoutCreateInfo, nullptr,
            &descriptorSetLayout);
}

void Material::createDescriptorPool(const Engine *engine) {
    std::array<VkDescriptorPoolSize, 2> poolSizes = {{
         {.descriptorCount = Engine::NUM_IMAGES_IN_SWAPCHAIN,
                 .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,},
         {.descriptorCount = Engine::NUM_IMAGES_IN_SWAPCHAIN,
                 .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,},
    }};
    VkDescriptorPoolCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data(),
            .maxSets = static_cast<uint32_t>(Engine::NUM_IMAGES_IN_SWAPCHAIN),
    };

    vkCreateDescriptorPool(engine->vkDevice, &createInfo, nullptr, &descriptorPool);
}

void Material::createDescriptorSets(const Engine *engine, const Texture *texture) {
    std::vector<VkDescriptorSetLayout> layouts(Engine::NUM_IMAGES_IN_SWAPCHAIN, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = Engine::NUM_IMAGES_IN_SWAPCHAIN,
            .pSetLayouts = layouts.data(),
    };
    descriptorSets.resize(Engine::NUM_IMAGES_IN_SWAPCHAIN);
    vkAllocateDescriptorSets(engine->vkDevice, &allocateInfo, descriptorSets.data());

    for(size_t i = 0; i < Engine::NUM_IMAGES_IN_SWAPCHAIN; i++) {
        VkDescriptorBufferInfo bufferInfo{
                .buffer = engine->uniformBuffers[i],
                .offset = 0,
                .range = sizeof(UniformBuffer),
        };
        VkDescriptorImageInfo imageInfo{
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .imageView = texture->imageView,
                .sampler = texture->sampler,
        };

        std::array<VkWriteDescriptorSet, 2> writeDescriptorSets = {{
           {
               .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
               .pNext = nullptr,
               .dstSet = descriptorSets[i],
               .dstBinding = 0,
               .dstArrayElement = 0,
               .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
               .descriptorCount = 1,
               .pBufferInfo = &bufferInfo,
               .pImageInfo = nullptr,
               .pTexelBufferView = nullptr,
           },
           {
               .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
               .pNext = nullptr,
               .dstSet = descriptorSets[i],
               .dstBinding = 1,
               .dstArrayElement = 0,
               .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
               .descriptorCount = 1,
               .pBufferInfo = nullptr,
               .pImageInfo = &imageInfo,
               .pTexelBufferView = nullptr,
           },
   }};
        vkUpdateDescriptorSets(engine->vkDevice, static_cast<uint32_t>(writeDescriptorSets.size()),
                               writeDescriptorSets.data(), 0, nullptr);
    }
}

void Material::createGraphicsPipelineLayout(const Engine *engine) {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = 1,
            .pSetLayouts = &descriptorSetLayout,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
    };
    vkCreatePipelineLayout(engine->vkDevice, &pipelineLayoutCreateInfo, nullptr,
                           &graphicsPipelineLayout);
}

void Material::createGraphicsPipeline(const Engine *engine) {
    VkShaderModule vertShaderModule = createShaderModule(engine, "shaders/shader.vert.spv");
    VkShaderModule fragShaderModule = createShaderModule(engine, "shaders/shader.frag.spv");

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertShaderModule,
            .pName = "main",
            .flags = 0,
            .pSpecializationInfo = nullptr,
    };
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragShaderModule,
            .pName = "main",
            .flags = 0,
            .pSpecializationInfo = nullptr,
    };
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data(),
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
    };

    VkViewport viewport{
            .x = 0,
            .y = 0,
            .width = static_cast<float>(
                    engine->physicalDeviceSurfaceCapabilities.currentExtent.width),
            .height = static_cast<float>(
                    engine->physicalDeviceSurfaceCapabilities.currentExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
    };
    VkRect2D scissor{
            .offset = {0, 0},
            .extent = engine->physicalDeviceSurfaceCapabilities.currentExtent,
    };
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor,
    };

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .lineWidth = 1.0f,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE, //
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .sampleShadingEnable = VK_FALSE,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE,
    };

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .minDepthBounds = 0.0f,
            .maxDepthBounds = 1.0f,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            .blendEnable = VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
    };

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachmentState,
            .blendConstants[0] = 0.0f,
            .blendConstants[1] = 0.0f,
            .blendConstants[2] = 0.0f,
            .blendConstants[3] = 0.0f,
    };

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputStateCreateInfo,
            .pInputAssemblyState = &inputAssemblyStateCreateInfo,
            .pViewportState = &viewportStateCreateInfo,
            .pRasterizationState = &rasterizationStateCreateInfo,
            .pMultisampleState = &multisampleStateCreateInfo,
            .pDepthStencilState = &depthStencilStateCreateInfo,
            .pColorBlendState = &colorBlendStateCreateInfo,
            .pDynamicState = nullptr,
            .layout = graphicsPipelineLayout,
            .renderPass = engine->renderPass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1,
    };
    vkCreateGraphicsPipelines(engine->vkDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr,
                              &graphicsPipeline);

    vkDestroyShaderModule(engine->vkDevice, vertShaderModule, nullptr);
    vkDestroyShaderModule(engine->vkDevice, fragShaderModule, nullptr);
}


VkShaderModule Material::createShaderModule(const Engine *engine, const char *fileName) {
    AAsset* shaderFile = AAssetManager_open(engine->activity->assetManager,
                                            fileName, AASSET_MODE_BUFFER);
    size_t shaderFileLen = AAsset_getLength(shaderFile);
    std::vector<char> content(shaderFileLen);
    AAsset_read(shaderFile, content.data(), shaderFileLen);
    AAsset_close(shaderFile);

    VkShaderModuleCreateInfo vertShaderModuleCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = content.size(),
            .pCode = reinterpret_cast<const uint32_t *>(content.data()),
    };
    VkShaderModule shaderModule;
    vkCreateShaderModule(engine->vkDevice, &vertShaderModuleCreateInfo, nullptr, &shaderModule);
    return shaderModule;
}