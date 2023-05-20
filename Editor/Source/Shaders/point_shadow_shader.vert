#version 450
layout(location = 0) in vec3 vPos;
layout(location = 4) in uint modelIdx;

layout(set = 0, binding = 0) readonly buffer ASD
{
	mat4 models[10];
};

void main()
{
	gl_Position = models[modelIdx] * vec4(vPos, 1.0f);
}