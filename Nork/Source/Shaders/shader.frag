#version 450
#extension GL_EXT_nonuniform_qualifier: require // required only if we index samplers non-uniformly (eg. using InstanceID)

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 worldPos;
layout(location = 3) nonuniformEXT flat in uint imgIdx;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec4 outColor;

layout(set = 0, binding = 2) uniform sampler2D[] texSampler;

void main() {
    // outPos = worldPos;
    outColor = texture(texSampler[imgIdx], fragTexCoord * 2.0);
    //outColor = vec4(1.0);
}