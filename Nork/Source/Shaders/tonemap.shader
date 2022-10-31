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
out vec4 fColor;
uniform sampler2D tex;

void main()
{
	float exposure = 1.0f;
	float gamma = 1.0f;
	vec3 col = texture(tex, fTex).rgb;
	fColor = vec4(col, 1);

	fColor = vec4(vec3(1.0f) - exp(-fColor.rgb * exposure), 1.0f);
	fColor.rgb = pow(fColor.rgb, vec3(1.0f / gamma));
}