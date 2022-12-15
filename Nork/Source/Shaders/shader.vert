#version 450

layout(binding = 0) uniform UniformBufferObject {
    uint idx;
} ubo;
struct SSBO_{
    mat4 VP;
    float offs;
};
readonly layout(binding = 1) buffer SSBO {
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
	mat4 render_matrix;
} PushConstants;

void main() {
    imgIdx = uint(mod(gl_InstanceIndex, MAX_IMG_ARR_SIZE));  
    SSBO_ d = ssbo.d[ubo.idx];
    vec3 translate = vec3(gl_InstanceIndex / 200.0, mod(gl_InstanceIndex, 200.0), 0) - vec3(100, 100, 0);
    translate *= vec3(vec2(d.offs), 0);
    //translate.z = -gl_InstanceIndex * 0.001 * 0.001 * 0.001;
    vec4 wPos = d.VP * vec4(inPosition + translate, 1.0);
    // vec4 wPos = PushConstants.render_matrix * vec4(inPosition + translate, 1.0);
    //vec4 wPos = vec4(inPosition.xy, 0.0, 1.0);
    worldPos = wPos.xyz;
    gl_Position = wPos;
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}