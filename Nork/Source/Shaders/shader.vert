#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 VP;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 worldPos;

void main() {
    vec4 wPos = ubo.VP * vec4(inPosition, 1.0);
    worldPos = wPos.xyz;
    gl_Position = wPos;
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}