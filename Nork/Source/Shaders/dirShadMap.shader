#type vertex

#version 330 core
#extension ARB_shader_draw_parameters : require

layout(location = 0) in vec3 vPos;

uniform mat4 VP;
layout(std140, binding = 5) uniform asd5
{
	mat4 models[1];
};
layout(std140, binding = 7) uniform asd8
{
	uvec4 modelMatIndexes[1];
};
uniform int instanced;

void main()
{
	uint drawIdx = gl_BaseInstance + gl_InstanceID;
	uint modelIdx = modelMatIndexes[drawIdx / 2][(drawIdx % 2) * 2];
	mat4 _model = models[modelIdx];

	gl_Position = VP * _model * vec4(vPos, 1.0f);
}

#type fragment

#version 330 core

void main()
{
	gl_FragDepth = gl_FragCoord.z;
}