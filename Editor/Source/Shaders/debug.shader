#type vertex
#version 430 core

layout(location = 0) in vec3 vPos;

uniform mat4 VP;

void main()
{
	gl_Position = vec4(vPos, 1.0f);
}

#type fragment
#version 430 core

out vec4 fColor;

struct Config
{
	mat4 V;
	mat4 iP;
	uint lightCount;
	uint atomicCounter;
	uvec2 cullRes;
};

layout(std430, binding = 11) buffer BUF2
{
	uvec2 ranges[];
};
layout(std430, binding = 12) buffer BUF3
{
	Config config;
};

layout(std140, binding = 0) uniform asd0
{
	uint dLightCount, dShadowCount;
	uint pLightCount, pShadowCount;
};

void main()
{
	float x = gl_FragCoord.x / 1920.0f;
	float y = gl_FragCoord.y / 1080.0f;

	int i = int(x * config.cullRes.x);
	int j = int(y * config.cullRes.y);

	uvec2 val = ranges[j * config.cullRes.x + i];
	float amount = float(val.y);

	float str = amount / float(pLightCount);
	fColor = vec4(str, str, str, 1.0f);
}