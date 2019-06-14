//
// Created by andys on 6/4/2019.
//

#include <engine.h>

float AnimController::interpolate(std::vector<glm::vec2> &curve) {
    int i;
    for(i = 0; i < curve.size(); i++) {
        /* curve[i].x <= curve[i+1].x */
        if(t < curve[i].x) {
            break;
        }
    }
    /* 0 <= i <= curve.size() */
    if(i < 2) {
        return curve[i-1].y;
    } else if(i > curve.size()-2) {
        return curve[i-1].y;
    }

    float s = (t - curve[i-1].x) / (curve[i].x - curve[i-1].x); /* 0 <= s < 1 */
    return glm::catmullRom(curve[i-2], curve[i-1], curve[i], curve[i+1], s).y;
}

glm::mat4 AnimController::advance(float dt) {
    t += dt;
    if(t >= tMax) {
        t = 0.0f;
    }
    /* 0 <= t < tMax */

    glm::mat4 result(1.0f); /* identity */
    glm::vec3 T, R, S;

    T.x = interpolate(posX);
    T.y = interpolate(posY);
    T.z = interpolate(posZ);
    R.x = interpolate(rotX);
    R.y = interpolate(rotY);
    R.z = interpolate(rotZ);
    S.x = interpolate(scaleX);
    S.y = interpolate(scaleY);
    S.z = interpolate(scaleZ);

    /*__android_log_print(ANDROID_LOG_INFO, "main",
                    "S: (%f, %f, %f)", S.x, S.y, S.z);*/

    /* result = T * Rx * Ry * Rz * S */
    result = glm::translate(result, T);
    result = glm::rotate(result, glm::radians(R.x), glm::vec3(1, 0, 0));
    result = glm::rotate(result, glm::radians(R.y), glm::vec3(0, 1, 0));
    result = glm::rotate(result, glm::radians(R.z), glm::vec3(0, 0, 1));
    result = glm::scale(result, S);

    return result;
}

void Engine::updateUniformBuffer() {
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
    vkMapMemory(vkDevice, uniformBuffersMemory, 0, sizeof(UniformBuffer), 0, &data);
    memcpy(data, &ubo, sizeof(UniformBuffer));
    vkUnmapMemory(vkDevice, uniformBuffersMemory);
}