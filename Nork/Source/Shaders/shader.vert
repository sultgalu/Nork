#version 450

//#extension GL_ARB_shader_draw_parameters : require

layout(set = 0, binding = 0) uniform UniformBufferObject {
    uint[10] idx;
} ubo;
struct SSBO_{
    mat4 model;
};
layout(set = 0, binding = 1) readonly buffer SSBO {
    SSBO_[] d;
} ssbo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 worldPos;
layout(location = 3) flat out uint imgIdx;

layout(push_constant) uniform constants
{
	mat4 VP;
} PushConstants;

void main() {
    imgIdx = gl_InstanceIndex % MAX_IMG_ARR_SIZE; // gl_BaseInstanceARB; //gl_DrawIDARB gl_InstanceIndex% MAX_IMG_ARR_SIZE;
    SSBO_ d = ssbo.d[ubo.idx[gl_InstanceIndex]];
    // float size = 1;
    //vec3 translate = vec3(gl_InstanceIndex / size, mod(gl_InstanceIndex, size), 0) - vec3(size / 2, size / 2, 0);
    //translate *= vec3(vec2(d.offs), 0);
    //translate.z = -gl_InstanceIndex * 0.001 * 0.001 * 0.001;
    vec4 wPos = PushConstants.VP * d.model * vec4(inPosition, 1.0);
    // vec4 wPos = PushConstants.render_matrix * vec4(inPosition + translate, 1.0);
    //vec4 wPos = vec4(inPosition.xy, 0.0, 1.0);
    worldPos = wPos.xyz;
    gl_Position = wPos;
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}