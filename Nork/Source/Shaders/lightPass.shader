#type vertex

#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 texCoord;

void main()
{
	gl_Position = vec4(aPos.x, aPos.y, 0.0f, 1.0f);
	texCoord = aTexCoord;
}

#type fragment

#version 430 core

layout(location = 0) out vec4 fColor;

// --------- GLOBAL ----------

struct DirLight
{
	vec3 direction;
	vec4 color;
};

struct DirShadow
{
	mat4 VP;
	float bias, biasMin, pcfSize;
	int idx;
};

struct PointLight
{
	vec3 position;
	vec4 color;

	float linear, quadratic;
};

struct PointShadow
{
	float bias, biasMin;
	int blur;
	float radius, far, near;
	int idx;
	float dummy;
};

struct CullConfig
{
	mat4 V;
	mat4 iP;
	uint lightCount;
	uint atomicCounter;
	uvec2 cullRes;
};

layout(std140, binding = 0) uniform asd0
{
	uint dLightCount, dShadowCount;
	uint pLightCount, pShadowCount;
};
layout(std140, binding = 1) uniform asd1
{
	DirLight dLs[10];
};
layout(std140, binding = 2) uniform asd2
{
	DirShadow dLSs[10];
};
layout(std140, binding = 3) uniform asd3
{
	PointLight pLs[10];
};
layout(std140, binding = 4) uniform asd4
{
	PointShadow pLSs[10];
};
//layout(std430, binding = 10) buffer Buf0
//{
//	uint pLightIndicies[];
//};
//layout(std430, binding = 11) buffer Buf
//{
//	uvec2 ranges[];
//};
//layout(std430, binding = 12) buffer BUF3
//{
//	CullConfig cullConfig;
//};
// --------- GLOBAL ----------

struct Materials
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float specularExponent;
};

in vec2 texCoord;

uniform vec3 viewPos;

uniform sampler2D gPos;
uniform sampler2D gDiff;
uniform sampler2D gNorm;
uniform sampler2D gSpec;

uniform sampler2DShadow dirShadowMaps[5];
uniform samplerCubeShadow pointShadowMaps[15];

vec3 dLight(DirLight light, Materials material, vec3 normal, vec3 viewDir);
vec3 dLightShadow(DirLight light, Materials material, vec3 normal, vec3 viewDir, DirShadow shadow, vec3 worldPos);

vec3 pLight(PointLight light, Materials material, vec3 worldPos, vec3 normal, vec3 viewDir);
vec3 pLightShadow(PointLight light, Materials material, vec3 normal, vec3 viewDir, PointShadow shadow, vec3 worldPos);

void main()
{
	/*float x = gl_FragCoord.x / 1920.0f;
	float y = gl_FragCoord.y / 1080.0f;

	int i = int(x * cullConfig.cullRes.x);
	int j = int(y * cullConfig.cullRes.y);

	uvec2 range = ranges[j * int(cullConfig.cullRes.x) + i];*/
	//----------------------------NEW-----------------------------
	vec3 worldPos = texture(gPos, texCoord).rgb;
	vec3 diff = texture(gDiff, texCoord).rgb;
	vec3 normal = texture(gNorm, texCoord).rgb;
	vec2 spec = texture(gSpec, texCoord).rg;

	vec3 viewDir = normalize(viewPos - worldPos);

	Materials material;
	material.ambient = diff.rgb;
	material.diffuse = diff.rgb;
	material.specular = material.diffuse * spec.r;
	material.specularExponent = spec.g;
	
	vec3 result = vec3(0.0f);
	for (uint i = uint(0); i < dShadowCount; i++)
	{
		result += dLightShadow(dLs[i], material, normal, viewDir, dLSs[i], worldPos);
	}
	for (uint i = dShadowCount; i < dLightCount; i++)
	{
		result += dLight(dLs[i], material, normal, viewDir);
	}
	
	//for (uint i = range.x; i < range.x + range.y; i++)
	//{
	//	uint idx = pLightIndicies[i];
	//	if (idx < pShadowCount) // plight has shadow
	//	{
	//		result += pLightShadow(pLs[idx], material, normal, viewDir, pLSs[idx], worldPos);
	//	}
	//	else
	//	{
	//		result += pLight(pLs[idx], material, worldPos, normal, viewDir);
	//	}
	//}
	for (uint i = uint(0); i < pShadowCount; i++)
	{
		result += pLightShadow(pLs[i], material, normal, viewDir, pLSs[i], worldPos);
	}
	if (pShadowCount < pLightCount) // if left out, weird rendering happens (can't handle for loop below)
	{
		for (uint i = pShadowCount; i < pLightCount; i++)
		{
			result += pLight(pLs[i], material, worldPos, normal, viewDir);
		}
	}
	fColor = vec4(result, 1.0f);
	// fColor = vec4(texture(dirShadowMaps[0], texCoord).rgb, 1.0f);
	// fColor = vec4(diff, 1);
}

vec3 dLight(DirLight light, Materials material, vec3 normal, vec3 viewDir)
{
	vec3 ambient = light.color.rgb * material.ambient;

	vec3 lightDir = -normalize(light.direction); // acts now as a position pointing towards the origin (otherwise should * (-1)) 
	float diffAngle = max(dot(lightDir, normal), 0.0f);
	vec3 diffuse = light.color.rgb * diffAngle * material.diffuse;

	vec3 halfwayDir = normalize(viewDir + lightDir);
	float angle = pow(max(dot(halfwayDir, normal), 0.0f), material.specularExponent);
	vec3 specular = light.color.rgb * angle * material.specular;

	return 0.2f * ambient + diffuse + specular;
}
float dShadow(DirShadow shadow, float bias, vec4 lightView)
{
	vec3 fragPos = lightView.xyz / lightView.w; // clipping
	fragPos = (fragPos + 1.0f) / 2.0f; // [-1;1] -> [0;1]
	fragPos.z -= bias;
	return texture(dirShadowMaps[shadow.idx], fragPos.xyz);
}
vec3 dLightShadow(DirLight light, Materials material, vec3 normal, vec3 viewDir, DirShadow shadow, vec3 worldPos)
{
	vec3 ambient = light.color.rgb * material.ambient;

	vec3 lightDir = -normalize(light.direction); // acts now as a position pointing towards the origin (otherwise should * (-1)) 
	float diffAngle = max(dot(lightDir, normal), 0.0f);
	vec3 diffuse = light.color.rgb * diffAngle * material.diffuse;

	vec3 halfwayDir = normalize(viewDir + lightDir);
	float angle = pow(max(dot(halfwayDir, normal), 0.0f), material.specularExponent);
	vec3 specular = light.color.rgb * angle * material.specular;

	vec4 lightView = shadow.VP * vec4(worldPos, 1.0f);
	float bias = max(shadow.bias * (1.0f - dot(normal, lightDir)), shadow.biasMin); // for huge angle, bias = 0.05f (perpendicular)
	float shad = dShadow(shadow, bias, lightView);

	return 0.2f * ambient + shad * (diffuse + specular);
}

float CalcLuminosity(vec3 fromPos, vec3 toPos, float linear, float quadratic)
{
	float distance = length(toPos - fromPos);
	float attenuation = 1.0f / (1.0f + linear * distance + quadratic * distance * distance);
	//if (attenuation <= 0.001f) attenuation = 0.0f;
	return attenuation;
}

vec3 pLight(PointLight light, Materials material, vec3 worldPos, vec3 normal, vec3 viewDir)
{
	vec3 ambient = light.color.rgb * material.ambient;

	vec3 lightDir = normalize(light.position - worldPos);
	float diffAngle = max(dot(lightDir, normal), 0.0f);
	vec3 diffuse = light.color.rgb * diffAngle * material.diffuse;

	vec3 halfwayDir = normalize(viewDir + lightDir);
	float angle = pow(max(dot(halfwayDir, normal), 0.0f), material.specularExponent);
	vec3 specular = light.color.rgb * angle * material.specular;

	float attenuation = CalcLuminosity(light.position, worldPos, light.linear, light.quadratic);
	return attenuation * (ambient + diffuse + specular);
}
float pShadow(PointShadow shadow, float bias, vec3 worldPos, vec3 lightPos)
{
	vec3 direction = worldPos - lightPos;
	return texture(pointShadowMaps[shadow.idx], vec4(direction.xyz, length(direction) / shadow.far - bias));
}
vec3 pLightShadow(PointLight light, Materials material, vec3 normal, vec3 viewDir, PointShadow shadow, vec3 worldPos)
{
	vec3 ambient = light.color.rgb * material.ambient;

	vec3 lightDir = normalize(light.position - worldPos);
	float diffAngle = max(dot(lightDir, normal), 0.0f);
	vec3 diffuse = light.color.rgb * diffAngle * material.diffuse;

	vec3 halfwayDir = normalize(viewDir + lightDir);
	float angle = pow(max(dot(halfwayDir, normal), 0.0f), material.specularExponent);
	vec3 specular = light.color.rgb * angle * material.specular;

	float bias = max(shadow.bias * (1.0f - dot(normal, lightDir)), shadow.biasMin); // for huge angle, bias = 0.05f (perpendicular)
	float shad = pShadow(shadow, bias, worldPos, light.position);

	float attenuation = CalcLuminosity(light.position, worldPos, light.linear, light.quadratic);
	return attenuation * (ambient + shad * (diffuse + specular));
}