#type vertex

#version 330 core
#extension ARB_shader_draw_parameters : require

layout(location = 0) in vec3 vPos;

uniform mat4 VP;
uniform mat4 model;
layout(std140, binding = 5) uniform asd5
{
	mat4 models[1];
};
uniform int instanced;

void main()
{
	gl_Position = VP * models[gl_BaseInstance + gl_InstanceID] * vec4(vPos, 1.0f);
}

#type fragment

#version 330 core

void main()
{
	gl_FragDepth = gl_FragCoord.z;
}