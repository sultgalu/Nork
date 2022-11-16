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
#extension ARB_bindless_texture : require

layout(location = 0) out vec4 fColor;

// --------- GLOBAL ----------

struct DirLight
{
	vec3 direction;
	float outOfProj;
	vec4 color; // ambient
	vec4 color2;
	mat4 VP;
};

struct DirShadow
{
	float bias, biasMin;
	sampler2DShadow shadMap;
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
	float near, far;
	samplerCubeShadow shadMap;
};

struct CullConfig
{
	mat4 V;
	mat4 iP;
	uint lightCount;
	uint atomicCounter;
	uvec2 cullRes;
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
layout(std140, binding = 8) uniform asd5
{
	uint dLightCount, dShadowCount;
	uint pad0, pad1;
	uvec4 dirIdxs[1];
};
layout(std140, binding = 9) uniform asd6
{
	uint pLightCount, pShadowCount;
	uint pad2, pad3;
	uvec4 pointIdxs[1];
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
	float roughness;
	float metallic;
	vec3 albedo;
};

in vec2 texCoord;

uniform vec3 viewPos;

uniform sampler2D gPos;
uniform sampler2D gDiff;
uniform sampler2D gNorm;
uniform sampler2D gSpec;

vec3 dLightShadow(DirLight light, Materials material, vec3 normal, vec3 viewDir, DirShadow shadow, vec3 worldPos);

float dShadow(DirLight light, DirShadow shadow, vec3 normal, vec3 worldPos);
float pShadow(PointShadow shadow, vec3 normal, vec3 lightDir, vec3 distance);
vec3 pbr(vec3 lightDir, vec3 color, Materials material, vec3 worldPos, vec3 normal, vec3 viewDir, float attenuation);
vec3 pLightShadow(PointLight light, Materials material, vec3 normal, vec3 viewDir, PointShadow shadow, vec3 worldPos);

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

vec3 F0 = vec3(0.04);;

void main()
{
	vec4 pos = texture(gPos, texCoord).rgba;
	if (pos.a == 0.0f)
		discard;
	vec3 worldPos = pos.rgb;
	vec3 normal = texture(gNorm, texCoord).rgb;

	Materials material;
	material.albedo = texture(gDiff, texCoord).rgb;
	vec2 roughnessMetalness = texture(gSpec, texCoord).rg;
	material.roughness = roughnessMetalness.r;
	material.metallic = roughnessMetalness.g;

	vec3 viewDir = normalize(worldPos - viewPos);

	F0 = mix(F0, material.albedo, material.metallic);

	vec3 Lo = vec3(0.0);
	for (uint i = uint(0); i < dShadowCount; i++)
	{
		uint j = i * 2;
		uint li = dirIdxs[j / 4][j % 4];
		uint si = dirIdxs[j / 4][j % 4 + 1];
		float shad = dShadow(dLs[li], dLSs[si], normal, worldPos);
		Lo += shad * pbr(normalize(dLs[li].direction), dLs[li].color.rgb, material, worldPos, normal, viewDir, 1.0f);
	}
	for (uint i = dShadowCount * 2; i < dShadowCount * 2 + (dLightCount - dShadowCount); i++)
	{
		uint li = dirIdxs[i / 4][i % 4];
		Lo += pbr(normalize(dLs[li].direction), dLs[li].color.rgb, material, worldPos, normal, viewDir, 1.0f);
	}
	
	for (uint i = uint(0); i < pShadowCount; i++)
	{
		uint j = i * 2;
		uint li = pointIdxs[j / 4][j % 4];
		uint si = pointIdxs[j / 4][j % 4 + 1];

		float distance = length(pLs[li].position - worldPos);
		float attenuation = 1.0 / (distance * distance);
		vec3 lightDir = normalize(worldPos - pLs[li].position);
		float shad = pShadow(pLSs[si], normal, lightDir, worldPos - pLs[li].position);
		Lo += shad * pbr(lightDir, pLs[li].color.rgb, material, worldPos, normal, viewDir, attenuation);
	}
	for (uint i = pShadowCount * 2; i < pShadowCount * 2 + (pLightCount - pShadowCount); i++)
	{
		uint li = pointIdxs[i / 4][i % 4];
		float distance = length(pLs[li].position - worldPos);
		float attenuation = 1.0 / (distance * distance); // inverse-square law. More physically correct but gives less manual control
		vec3 lightDir = normalize(worldPos - pLs[li].position);
		Lo += pbr(lightDir, pLs[li].color.rgb, material, worldPos, normal, viewDir, attenuation);
	}
	vec3 ambient = vec3(0.03) * material.albedo; // *ao
	vec3 color = ambient + Lo;

	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0 / 2.2));
	fColor = vec4(color, 1.0f);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}
float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}
vec3 pbr(vec3 lightDir, vec3 color, Materials material, vec3 worldPos, vec3 normal, vec3 viewDir, float attenuation)
{
	// translate ...
	float roughness = material.roughness;
	float metallic = material.metallic;
	vec3 albedo = material.albedo;
	vec3 N = normal;
	vec3 V = -viewDir;

	vec3 L = -lightDir;
	vec3 H = normalize(V + L);
	vec3 radiance = color * attenuation;

	// cook-torrance brdf
	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
	vec3 specular = numerator / denominator;

	// add to outgoing radiance Lo
	float NdotL = max(dot(N, L), 0.0);
	return (kD * albedo / PI + specular) * radiance * NdotL;
}

float clip(vec3 fragPos, float clippedVal)
{
	return (pow(fragPos.x, 2.0) > 1 || pow(fragPos.y, 2.0) > 1 || pow(fragPos.z, 2.0) > 1) ? clippedVal : 1;
}
float dShadow(DirLight light, DirShadow shadow, vec3 normal, vec3 worldPos)
{
	vec4 lightView = light.VP * vec4(worldPos, 1.0f);
	vec3 fragPos = lightView.xyz / lightView.w; // clipping
	float bias = max(shadow.bias * (1.0f - dot(normal, normalize(-light.direction))), shadow.biasMin); // for huge angle, bias = 0.05f (perpendicular)

	fragPos = (fragPos + 1.0f) / 2.0f; // [-1;1] -> [0;1]
	fragPos.z -= bias;
	return clip(fragPos, light.outOfProj) * texture(shadow.shadMap, fragPos.xyz);
}
float pShadow(PointShadow shadow, vec3 normal, vec3 lightDir, vec3 distance)
{
	float bias = max(shadow.bias * (1.0f - dot(normal, -lightDir)), shadow.biasMin); // for huge angle, bias = 0.05f (perpendicular)
	return texture(shadow.shadMap, vec4(distance.xyz, length(distance) / shadow.far - bias));
}
// can be used instead of inverse-square for better manual control
float CalcLuminosity(vec3 fromPos, vec3 toPos, float linear, float quadratic)
{
	float distance = length(toPos - fromPos);
	float attenuation = 1.0f / (1.0f + linear * distance + quadratic * distance * distance);
	return attenuation;
}