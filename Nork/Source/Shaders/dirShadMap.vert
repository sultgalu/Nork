#version 450
layout(location = 0) in vec3 vPos;

layout(push_constant) uniform constants
{
	mat4 VP;
} PushConstants;

layout(set = 0, binding = 1) readonly buffer asd5
{
	mat4 models[10];
};
layout(set = 0, binding = 0) uniform asd8
{
	uvec4 modelMatIndexes[10]; // uvec2 and uvec4 have the same stride, so using the latter is more memory efficient, just a little more complicated in indexing
};

void main()
{
	uint drawIdx = gl_InstanceIndex; // gl_BaseInstance + gl_InstanceID;
	uint modelIdx = modelMatIndexes[drawIdx / 2][(drawIdx % 2) * 2];
	mat4 _model = models[modelIdx];

	gl_Position = PushConstants.VP * _model * vec4(vPos, 1.0f);
}