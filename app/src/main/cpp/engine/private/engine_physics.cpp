//
// Created by andys on 6/4/2019.
//

#include <engine.h>

void Engine::updateUniformBuffer(uint32_t imageIndex) {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float,
            std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBuffer ubo{
            .model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                                 glm::vec3(0.0f, 0.0f, 1.0f)),
            .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, 0.0f, 1.0f)),
            .proj = glm::perspective(glm::radians(45.0f),
                                     physicalDeviceSurfaceCapabilities.currentExtent.width /
                                     static_cast<float>(physicalDeviceSurfaceCapabilities.currentExtent.height),
                                     0.1f, 10.0f),
    };

    /* Vulkan NDC y-axis points downwards, while OpenGL's pointing upwards
     *
     * projection matrix determines handedness, so Vulkan is right-handedness instead of OpenGL's
     * left-handedness
     */
    ubo.proj[1][1] *= -1;

    void *data;
    vkMapMemory(vkDevice, uniformBuffersMemory[imageIndex], 0, sizeof(UniformBuffer), 0, &data);
    memcpy(data, &ubo, sizeof(UniformBuffer));
    vkUnmapMemory(vkDevice, uniformBuffersMemory[imageIndex]);
}