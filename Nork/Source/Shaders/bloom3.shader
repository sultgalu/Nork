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

out vec3 fColor;
layout(bindless_sampler) uniform sampler2D tex;
in vec2 fTex;
// void main()
// {
// 	fColor = texture(tex, fTex).rgb;
// }
void main()
{
	ivec2 texSize = textureSize(tex, 0);
	int dim = 2;
	vec3 col = vec3(0);
	int divider = 0;
	for (int i = -dim; i <= dim; i++)
	{
		for (int j = -dim; j <= dim; j++)
		{
			vec2 texCoord = vec2(fTex.x + float(i) / texSize.x, fTex.y + float(j) / texSize.y);
			int multiplier = (dim+1) * (dim+1) - ((abs(i) + 1) * (abs(j) + 1));
			//int multiplier = 1;
			col += texture(tex, texCoord).rgb * multiplier;
			divider += multiplier;
		}
	}
	//int count = (2 * dim + 1) * (2 * dim + 1);
	fColor = col / float(divider);
	//fColor = vec4(texture(tex, fTex).rgb, 1);
}

/*
uniform int dir = 0;
void main()
{
	ivec2 texSize = textureSize(tex, 0);
	int dim = 10;
	vec3 col = vec3(0);
	int divider = 0;

	for (int j = -dim; j <= dim; j++)
	{
		vec2 texCoord = vec2(fTex.x + dir * float(j) / texSize.x, fTex.y + (1-dir)*float(j) / texSize.y);

		int multiplier = dim - abs(j) + 1;
		// int multiplier = 1;
		col += texture(tex, texCoord).rgb * multiplier;
		divider += multiplier;
	}
	//int count = (2 * dim + 1) * (2 * dim + 1);
	fColor = 2 * col / float(divider;
	//fColor = vec4(texture(tex, fTex).rgb, 1);
}*/