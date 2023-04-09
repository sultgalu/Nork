#version 450
layout(location = 0) in vec3 vPos;

layout(push_constant) uniform constants
{
	mat4 VP;
} PushConstants;

void main() {
    gl_Position = PushConstants.VP * vec4(vPos, 1.0);
}