#version 450
#extension GL_EXT_nonuniform_qualifier: require // required only if we index samplers non-uniformly

//#define DEFERRED
//#define FORWARD
//#define UNLIT

layout(location = 0) out vec4 fColor;
#ifndef DEFERRED
layout(location = 1) out vec4 fEmissive; // 3 used
#endif

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
	uint shadMap;
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
	uint shadMap;
};

struct CullConfig
{
	mat4 V;
	mat4 iP;
	uint lightCount;
	uint atomicCounter;
	uvec2 cullRes;
};
struct Material
{
	uint baseColor_normal;
	uint metallicRoughness_occlusion;
	float roughnessFactor;
	float metallicFactor;

	vec3 emissiveFactor;
  uint emissive;

	vec4 baseColorFactor;

	float alphaCutoff;
	float padding[3];
};
layout(std140, set = 2, binding = 0) uniform asd1
{
	DirLight dLs[10];
};
layout(std140, set = 2, binding = 1) uniform asd2
{
	DirShadow dLSs[10];
};
layout(std140, set = 2, binding = 2) uniform asd3
{
	PointLight pLs[10];
};
layout(std140, set = 2, binding = 3) uniform asd4
{
	PointShadow pLSs[10];
};
layout(std140, set = 2, binding = 4) uniform asd5
{
	uint dLightCount, dShadowCount;
	uint pad0, pad1; // TODO use point light indexes here not separately
	uvec4 dirIdxs[1];
};
layout(std140, set = 2, binding = 5) uniform asd6
{
	uint pLightCount, pShadowCount;
	uint pad2, pad3;
	uvec4 pointIdxs[1];
};
struct Materials
{
	vec4 albedo;
	float roughness;
	float metallic;
	float occlusion;
};
layout(set = 2, binding = 6) uniform sampler2DShadow[] shadowMaps;
layout(set = 2, binding = 7) uniform samplerCubeShadow[] shadowMapsCube;
layout(set = 2, binding = 8) readonly buffer asd7
{
	Material materials[1];
};
layout(push_constant) uniform constants
{
	layout(offset = 64) vec3 viewPos;
	float blend;
} PushConstants;

#ifdef DEFERRED
layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput gPosition;
layout(input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput gBaseColor;
layout(input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput gNormal;
layout(input_attachment_index = 3, set = 1, binding = 3) uniform subpassInput gMetallicRoughnessOcclusion;
#else // FORWARD, UNLIT
layout(location = 0) in vec3 worldPos;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in mat3 TBN;
layout(location = 5) nonuniformEXT flat in uint materialIdx;

layout(set = 0, binding = 3) uniform sampler2D[] textures;
#endif // DEFERRED, UNLIT

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

vec3 F0 = vec3(0.04);

void main_light()
{
	Materials material;
#ifdef DEFERRED
	vec4 pos = subpassLoad(gPosition).rgba;
	if (pos.a == 0.0f)
		discard;
	vec3 worldPos = pos.rgb;
	vec3 normal = subpassLoad(gNormal).rgb;

	material.albedo = vec4(subpassLoad(gBaseColor).rgb, 1.0);
	vec3 metallicRoughnessOcclusion = subpassLoad(gMetallicRoughnessOcclusion).rgb;
	material.roughness = metallicRoughnessOcclusion.r;
	material.metallic = metallicRoughnessOcclusion.g;
	material.occlusion = metallicRoughnessOcclusion.b;
#else // FORWARD
	Material material_ = materials[materialIdx];
	material.albedo = texture(textures[bitfieldExtract(material_.baseColor_normal, 0, 16)], texCoord).rgba;
	material.albedo = material.albedo * material_.baseColorFactor;
	if (material.albedo.a < material_.alphaCutoff)
		discard;
	vec2 metallicRoughness = texture(textures[bitfieldExtract(material_.metallicRoughness_occlusion, 0, 16)], texCoord).gb * vec2(material_.roughnessFactor, material_.metallicFactor);
	material.roughness = metallicRoughness.r;
	material.metallic = metallicRoughness.g;
	vec3 normal = texture(textures[bitfieldExtract(material_.baseColor_normal, 16, 16)], texCoord).rgb;
	normal = normal * 2.0f - 1.0f; // [0;1] -> [-1;1]
	normal = normalize(TBN * normal); // transforming from tangent-space -> world space
	material.occlusion = texture(textures[bitfieldExtract(material_.metallicRoughness_occlusion, 16, 16)], texCoord).r;
#endif // DEFERRED

	vec3 viewDir = normalize(worldPos - PushConstants.viewPos);

	F0 = mix(F0, material.albedo.rgb, material.metallic);

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
		float attenuation = 1.0 / (distance * distance);
		vec3 lightDir = normalize(worldPos - pLs[li].position);
		Lo += pbr(lightDir, pLs[li].color.rgb, material, worldPos, normal, viewDir, attenuation);
	}
	vec3 ambient = vec3(0.03) * material.albedo.rgb * material.occlusion;
	vec3 color = (ambient + Lo);
	#ifdef DEFERRED
	fColor = vec4(color, 1.0);
	#else // FORWARD
	fColor = vec4(color, PushConstants.blend == 0.0 ? 1.0 : material.albedo.a);
	fEmissive = vec4(material_.emissiveFactor * texture(textures[material_.emissive], texCoord).rgb, 1.0);
	#endif // DEFERRED
	// fColor = subpassLoad(gBaseColor);
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
	vec3 albedo = material.albedo.rgb;
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
	float bias = max(shadow.bias * (1.0f - dot(normal, normalize(-light.direction))), shadow.biasMin); // for huge angle, use biasmin (perpendicular)

	fragPos.xy = (fragPos.xy + 1.0f) / 2.0f; // [-1;1] -> [0;1]
	fragPos.z -= bias;
	return clip(fragPos, light.outOfProj) * texture(shadowMaps[shadow.shadMap], fragPos.xyz);
}
float pShadow(PointShadow shadow, vec3 normal, vec3 lightDir, vec3 distance)
{
	float distanceLen = length(distance);
	float bias = max(shadow.bias * distanceLen * (1.0f - dot(normal, -lightDir)), shadow.biasMin); // for huge angle, bias = 0.05f (perpendicular)
	return texture(shadowMapsCube[shadow.shadMap], vec4(distance.xyz, distanceLen / shadow.far - bias));
}

void main(){
#ifdef UNLIT
	Material material_ = materials[materialIdx];
	vec4 albedo = texture(textures[bitfieldExtract(material_.baseColor_normal, 0, 16)], texCoord).rgba;
	albedo *= material_.baseColorFactor;
	fColor = vec4(albedo.rgb, PushConstants.blend == 0.0 ? 1.0 : albedo.a);
	fEmissive.rgb = material_.emissiveFactor * texture(textures[material_.emissive], texCoord).rgb;
#else
main_light();
#endif
}
