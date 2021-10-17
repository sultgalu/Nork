#type vertex

#version 330 core

layout(location = 0) in vec3 vPos;

uniform mat4 VP;
uniform mat4 model;

void main()
{
	gl_Position = VP * model * vec4(vPos, 1.0f);
}

#type fragment

#version 330 core

void main()
{
	gl_FragDepth = gl_FragCoord.z;
}