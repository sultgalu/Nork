#version 450
#extension GL_EXT_nonuniform_qualifier: require // required only if we index samplers non-uniformly (eg. using InstanceID)

layout(location = 0) out vec4 position; // 3 used
layout(location = 1) out vec4 baseColor; // 3 used
layout(location = 2) out vec3 normal; // 3 used
layout(location = 3) out vec2 metallicRoughness; // 2 used

struct Material
{
	uint baseColor;
	uint normal;
	uint metallicRoughness;

	float metallicFactor;
	vec3 baseColorFactor;
	float roughnessFactor;
	float alphaCutoff;

  float padding[3];
};

layout(location = 0) in vec3 worldPos;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in mat3 TBN;
layout(location = 5) nonuniformEXT flat in Material material;

layout(set = 0, binding = 3) uniform sampler2D[] textures;

void main()
{
	position = vec4(worldPos, 1.0f);
	vec4 color = texture(textures[material.baseColor], texCoord).rgba;
	if (color.a < material.alphaCutoff)
		discard;
	baseColor.rgb = color.rgb * material.baseColorFactor;
	baseColor.a = 1.0;
	metallicRoughness = texture(textures[material.metallicRoughness], texCoord).gb * vec2(material.roughnessFactor, material.metallicFactor);

	vec3 norm = texture(textures[material.normal], texCoord).rgb;
	norm = norm * 2.0f - 1.0f; // [0;1] -> [-1;1]
	normal = normalize(TBN * norm); // transforming from tangent-space -> world space
	// baseColor = vec4(1.0, 0.0, 1.0, 1.0);
}