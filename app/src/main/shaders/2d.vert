#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform VertContant {
    layout(offset = 0) mat4 M;
} vc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = vec4((vc.M * vec4(inPosition.xy, 1.0f, 0.0f)).xy, 0.0f, 1.0f);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
