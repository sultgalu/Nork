#version 450
layout(location = 0) in vec3 vPos;
layout(location = 4) in uint modelIdx;

layout(push_constant) uniform constants
{
	mat4 VP;
} PushConstants;

layout(set = 0, binding = 0) readonly buffer asd5
{
	mat4 models[10];
};

void main()
{
	mat4 _model = models[modelIdx];

	gl_Position = PushConstants.VP * _model * vec4(vPos, 1.0f);
}