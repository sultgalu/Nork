#version 450
#extension GL_EXT_nonuniform_qualifier: require // required only if we index samplers non-uniformly (eg. using InstanceID)

layout(location = 0) out vec4 position; // 3 used
layout(location = 1) out vec4 baseColor; // 3 used
layout(location = 2) out vec3 normal; // 3 used
layout(location = 3) out vec3 metallicRoughnessOcclusion; // 3 used
layout(location = 4) out vec4 emissive; // 3 used

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

layout(location = 0) in vec3 worldPos;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in mat3 TBN;
layout(location = 5) nonuniformEXT flat in uint materialIdx;

layout(set = 0, binding = 3) uniform sampler2D[] textures;
layout(set = 2, binding = 8) readonly buffer asd7
{
	Material materials[1];
};

void main()
{
	Material material = materials[materialIdx];
	position = vec4(worldPos, 1.0f);
	vec4 color = texture(textures[bitfieldExtract(material.baseColor_normal, 0, 16)], texCoord).rgba;
	if (color.a < material.alphaCutoff)
		discard;
	baseColor = color * material.baseColorFactor;
	metallicRoughnessOcclusion.rg = texture(textures[bitfieldExtract(material.metallicRoughness_occlusion, 0, 16)], texCoord).gb * vec2(material.roughnessFactor, material.metallicFactor);
	metallicRoughnessOcclusion.b = texture(textures[bitfieldExtract(material.metallicRoughness_occlusion, 16, 16)], texCoord).r;
	emissive.rgb = material.emissiveFactor * texture(textures[material.emissive], texCoord).rgb;

	vec3 norm = texture(textures[bitfieldExtract(material.baseColor_normal, 16, 16)], texCoord).rgb;
	norm = norm * 2.0f - 1.0f; // [0;1] -> [-1;1]
	normal = normalize(TBN * norm); // transforming from tangent-space -> world space
	// baseColor = vec4(1.0, 0.0, 1.0, 1.0);
}