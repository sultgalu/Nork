#type vertex

#version 330 core

layout(location = 0) in vec3 vPos;

uniform mat4 VP;
uniform mat4 model;
layout(std140, binding = 5) uniform asd5
{
	mat4 models[1000 * 1000];
};
uniform int instanced;

void main()
{
	gl_Position = VP * (instanced > 0 ? models[gl_InstanceID] : model) * vec4(vPos, 1.0f);
}

#type fragment

#version 330 core

void main()
{
	gl_FragDepth = gl_FragCoord.z;
}