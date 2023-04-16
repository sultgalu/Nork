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
#extension ARB_bindless_texture : require

in vec2 fTex;
out vec3 fColor;
uniform sampler2D tex;

void main()
{
	vec3 col = texture(tex, fTex).rgb;
	//fColor = vec4(col, 1);
	float brightness = dot(col.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > 1)
		fColor = col;
	else
		fColor = vec3(0);
}