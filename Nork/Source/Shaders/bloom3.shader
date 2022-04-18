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

uniform int dim = 1;
uniform int step = 3;
uniform float power = 1.2f;
out vec3 fColor;
layout(bindless_sampler) uniform sampler2D tex;
in vec2 fTex;

void main()
{
	ivec2 texSize = textureSize(tex, 0);
	vec3 col = vec3(0);
	float divider = 0;
	for (int i = -dim; i <= dim; i++)
	{
		for (int j = -dim; j <= dim; j++)
		{
			vec2 texCoord = vec2(fTex.x + float(i * step) / texSize.x, fTex.y + float(j * step) / texSize.y);
			//float multiplier = (dim+1) * (dim+1) - ((abs(i) + 1) * (abs(j) + 1));
			float multiplier = 1;
			col += texture(tex, texCoord).rgb * multiplier;
			divider += multiplier;
		}
	}
	fColor = power * col / float(divider);
}