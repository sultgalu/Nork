#type vertex
#version 330 core

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec2 vTex;

out vec2 fTex;

void main()
{
	fTex = vTex;
	gl_Position = vec4(vPos, 0, 1);
}

#type fragment
#version 330 core

out vec4 fColor;
uniform sampler2D tex;
in vec2 fTex;

void main()
{
	fColor = vec4(texture(tex, fTex).rgb, 1);
}