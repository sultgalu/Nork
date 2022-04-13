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
#version 400 core
#extension ARB_bindless_texture : require

in vec2 fTex;
out vec3 fColor;
layout(bindless_sampler) uniform sampler2D tex;
layout(bindless_sampler) uniform sampler2D tex2;

void main()
{
	// vec4 pixels[8];
	// pixels[0] = textureGather(tex, fTex, 0);
	// pixels[1] = textureGather(tex, fTex, 1);
	// pixels[2] = textureGather(tex, fTex, 2);
	// pixels[3] = textureGather(tex, fTex, 3);
	// pixels[4 + 0] = textureGather(tex2, fTex, 0);
	// pixels[4 + 1] = textureGather(tex2, fTex, 1);
	// pixels[4 + 2] = textureGather(tex2, fTex, 2);
	// pixels[4 + 3] = textureGather(tex2, fTex, 3);
	// vec3 col1 = vec3(0);
	// for (int i = 0; i < 3; i++)
	// {
	// 	for (int j = 0; j < 4; j++)
	// 	{
	// 		col1[i] += pixels[i][j];
	// 	}
	// 	//col1[i] /= 4.0f;
	// }
	// vec3 col2 = vec3(0);
	// for (int i = 0; i < 3; i++)
	// {
	// 	for (int j = 0; j < 4; j++)
	// 	{
	// 		col2[i] += pixels[4 + i][j];
	// 	}
	// 	//col2[i] /= 4.0f;
	// }
	// fColor = vec3(col1 + col2);
	
	fColor = vec3((texture(tex, fTex).rgb + texture(tex2, fTex).rgb) * 1.0f);
	//fColor = vec3((texture(tex, fTex).rgb + texture(tex2, fTex).rgb) * 1.0f);
	// vec2 edgeDist = abs(fract(fTex * textureSize(tex, 0)));
	// if (edgeDist.x > 0.9f || edgeDist.x < 0.1f)
	// {
	// 	fColor += vec3(1, 0, 1);
	// }
	// if (edgeDist.y > 0.9f || edgeDist.y < 0.1f)
	// {
	// 	fColor += vec3(0, 1, 1);
	// }
	// if (abs(imagePosCenterity.x - 0.5) < 0.001 || abs(imagePosCenterity.y - 0.5) < 0.001)
	// {
	// }
	//else
	//{
	//	fColor = vec3((texture(tex, fTex).rgb + texture(tex2, fTex).rgb) * 1.0f);
	//}
}