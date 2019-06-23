#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_TEXTURES_PER_FRAME 8

layout(set = 0, binding = 1) uniform sampler2D texSampler[MAX_TEXTURES_PER_FRAME];
layout(push_constant) uniform FragConstant {
    layout(offset = 64) int texId;
} fc;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 sampledColor = texture(texSampler[fc.texId], fragTexCoord);
    outColor = vec4(fragColor * sampledColor.rgb, sampledColor.a);
}