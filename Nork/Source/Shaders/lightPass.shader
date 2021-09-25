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

#version 330 core

layout(location = 0) out vec4 fColor;

struct DirLight
{
	vec3 direction;
	vec4 color;
};

struct DirShadow
{
	mat4 VP;
	float bias, biasMin, pcfSize;
};

struct DirLightWithShad
{
	DirLight light;
	DirShadow shadow;
};

struct PointLight
{
	vec4 color;
	vec3 position;

	float linear, quadratic;
};

struct PointShadow
{
	float bias, biasMin, blur, radius, far;
};

struct PointLightWithShadow
{
	PointLight light;
	PointShadow shadow;
};

struct Materials
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec2 texCoord;

uniform vec3 viewPos;

uniform sampler2D gPos;
uniform sampler2D gDiff;
uniform sampler2D gNorm;

#define MAX_NUM_OF_DIR_LIGHTS 10
#define MAX_NUM_OF_DIR_SHADOWS 5
#define MAX_NUM_OF_P_LIGHTS 10
#define MAX_NUM_OF_P_SHADOWS 5

uniform sampler2D shadowMaps[MAX_NUM_OF_DIR_SHADOWS];
uniform samplerCube shadowMapsCube[MAX_NUM_OF_P_SHADOWS];

layout(std140) uniform dLightsUni
{
	float dLightCount, dShadowCount;
	float pLightCount, pShadowCount;
	DirLightWithShad dLSs[MAX_NUM_OF_DIR_SHADOWS];
	PointLightWithShadow pLSs[MAX_NUM_OF_P_SHADOWS];

	DirLight dLs[MAX_NUM_OF_DIR_LIGHTS];
	PointLight pLs[MAX_NUM_OF_P_LIGHTS];
};

vec3 dLight(DirLight light, Materials material, vec3 normal, vec3 viewDir);
vec3 dLightShadow(DirLight light, Materials material, vec3 normal, vec3 viewDir, DirShadow shadow, int samplerIdx, vec3 worldPos);

vec3 pLight(PointLight light, Materials material, vec3 worldPos, vec3 normal, vec3 viewDir);
vec3 pLightShadow(PointLight light, Materials material, vec3 normal, vec3 viewDir, PointShadow shadow, int samplerIdx, vec3 worldPos);

void main()
{
	vec3 worldPos = texture(gPos, texCoord).rgb;
	vec4 diff_spec = texture(gDiff, texCoord).rgba;
	vec3 normal = texture(gNorm, texCoord).rgb;

	vec3 viewDir = normalize(viewPos - worldPos);

	Materials material;
	material.ambient = diff_spec.rgb;
	material.diffuse = diff_spec.rgb;
	material.specular = material.diffuse * diff_spec.a;

	vec3 result = vec3(0.0f);
	for (int i = 0; i < int(dShadowCount); i++)
	{
		//result += vec3(dLSs[i].shadow.VP[0][0], 0, 0);
		result += dLightShadow(dLSs[i].light, material, normal, viewDir, dLSs[i].shadow, i, worldPos);
	}
	for (int i = int(dShadowCount); i < int(dLightCount); i++)
	{
		result += dLight(dLs[i], material, normal, viewDir);
	}
	for (int i = 0; i < int(pShadowCount); i++)
	{
		result += pLightShadow(pLSs[i].light, material, normal, viewDir, pLSs[i].shadow, i, worldPos);
	}
	for (int i = int(pShadowCount); i < int(pLightCount); i++)
	{
		result += pLight(pLs[i], material, worldPos, normal, viewDir);
	}
	fColor = vec4(result, 1.0f);
}

vec3 dLight(DirLight light, Materials material, vec3 normal, vec3 viewDir)
{
	vec3 ambient = light.color.rgb * material.ambient;

	vec3 lightDir = -normalize(light.direction); // acts now as a position pointing towards the origin (otherwise should * (-1)) 
	float diffAngle = max(dot(lightDir, normal), 0.0f);
	vec3 diffuse = light.color.rgb * diffAngle * material.diffuse;

	vec3 halfwayDir = normalize(viewDir + lightDir);
	float angle = pow(max(dot(halfwayDir, normal), 0.0f), 4 * 128);
	vec3 specular = light.color.rgb * angle * material.specular;

	return 0.2f * ambient + diffuse + specular;
}
float dShadow(DirShadow shadow, float bias, vec4 lightView, int shadowIdx)
{
	vec3 fragPos = lightView.xyz / lightView.w; // clipping
	fragPos = (fragPos + 1.0f) / 2.0f; // [-1;1] -> [0;1]

	float shad = 0.0f;
	vec2 texelPortion = 1.0f / textureSize(shadowMaps[shadowIdx], 0);
	for (float i = -shadow.pcfSize; i < shadow.pcfSize + 1; i++) // anti-aliasing shadows basically
	{
		for (float j = -shadow.pcfSize; j < shadow.pcfSize + 1; j++)
		{
			shad += (texture(shadowMaps[shadowIdx], fragPos.xy + (vec2(i * texelPortion.x, j * texelPortion.y))).r + bias < fragPos.z) ? 0.0f : 1.0f;
		}
	}
	return shad / pow(shadow.pcfSize * 2 + 1, 2);
}
vec3 dLightShadow(DirLight light, Materials material, vec3 normal, vec3 viewDir, DirShadow shadow, int samplerIdx, vec3 worldPos)
{
	vec3 ambient = light.color.rgb * material.ambient;

	vec3 lightDir = -normalize(light.direction); // acts now as a position pointing towards the origin (otherwise should * (-1)) 
	float diffAngle = max(dot(lightDir, normal), 0.0f);
	vec3 diffuse = light.color.rgb * diffAngle * material.diffuse;

	vec3 halfwayDir = normalize(viewDir + lightDir);
	float angle = pow(max(dot(halfwayDir, normal), 0.0f), 4 * 128);
	vec3 specular = light.color.rgb * angle * material.specular;

	vec4 lightView = shadow.VP * vec4(worldPos, 1.0f);
	float bias = max(shadow.bias * (1.0f - dot(normal, lightDir)), shadow.biasMin); // for huge angle, bias = 0.05f (perpendicular)
	float shad = dShadow(shadow, bias, lightView, samplerIdx);

	return 0.2f * ambient + shad * (diffuse + specular);
}

float CalcLuminosity(vec3 fromPos, vec3 toPos, float linear, float quadratic)
{
	float distance = length(toPos - fromPos);
	float attenuation = 1.0f / (1.0f + linear * distance + quadratic * distance * distance);
	return attenuation;
}

vec3 pLight(PointLight light, Materials material, vec3 worldPos, vec3 normal, vec3 viewDir)
{
	vec3 ambient = light.color.rgb * material.ambient;

	vec3 lightDir = normalize(light.position - worldPos);
	float diffAngle = max(dot(lightDir, normal), 0.0f);
	vec3 diffuse = light.color.rgb * diffAngle * material.diffuse;

	vec3 halfwayDir = normalize(viewDir + lightDir);
	float angle = pow(max(dot(halfwayDir, normal), 0.0f), 4 * 128);
	vec3 specular = light.color.rgb * angle * material.specular;

	float attenuation = CalcLuminosity(light.position, worldPos, light.linear, light.quadratic);
	return attenuation * (ambient + diffuse + specular);
}
float pShadow(PointShadow shadow, float bias, vec3 worldPos, vec3 lightPos, int samplerIdx)
{
	vec3 direction = worldPos - lightPos;
	float depth = length(direction);
	float closestDepth = texture(shadowMapsCube[samplerIdx], direction).r;

	const vec3 offs[20] = vec3[]
	(
		vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
		vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
		vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
		vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
		vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
		);

	float shad = 0.0f;
	bias *= shadow.far; // bias is given in [0;1] range, making it [0;far] 
	for (int i = 0; i < int(shadow.blur); i++)
	{
		float closestDepth = texture(shadowMapsCube[samplerIdx], direction + offs[i] * shadow.radius).r;
		closestDepth *= shadow.far; // [0;1] -> [0;farPlane] (we divided it in shadowShader)
		shad += closestDepth + bias < depth ? 0.0f : 1.0f;
	}
	return shad / (shadow.blur == 0 ? 1.0f : shadow.blur);
}
vec3 pLightShadow(PointLight light, Materials material, vec3 normal, vec3 viewDir, PointShadow shadow, int samplerIdx, vec3 worldPos)
{
	vec3 ambient = light.color.rgb * material.ambient;

	vec3 lightDir = normalize(light.position - worldPos);
	float diffAngle = max(dot(lightDir, normal), 0.0f);
	vec3 diffuse = light.color.rgb * diffAngle * material.diffuse;

	vec3 halfwayDir = normalize(viewDir + lightDir);
	float angle = pow(max(dot(halfwayDir, normal), 0.0f), 4 * 128);
	vec3 specular = light.color.rgb * angle * material.specular;

	float bias = max(shadow.bias * (1.0f - dot(normal, lightDir)), shadow.biasMin); // for huge angle, bias = 0.05f (perpendicular)
	float shad = pShadow(shadow, bias, worldPos, light.position, samplerIdx);

	float attenuation = CalcLuminosity(light.position, worldPos, light.linear, light.quadratic);
	return attenuation * (ambient + shad * (diffuse + specular));
}