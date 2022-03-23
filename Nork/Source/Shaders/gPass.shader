#type vertex

#version 330 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec3 vTangent;
// layout(location = 4) in vec3 vBiTangent; // = cross(vNormal, vTangent)

out vec3 worldPos;
out vec2 texCoord;
out vec4 lightViewPos; // new
out mat3 TBN;
out vec3 vNorm;

uniform mat4 model;
uniform mat4 VP;

layout(std140, binding = 5) uniform asd5
{
	mat4 models[1000 * 1000];
};

uniform int instanced;

void main()
{
	mat4 _model = instanced > 0 ? models[gl_InstanceID] : model;

	worldPos = (_model * vec4(vPos, 1.0f)).xyz;
	gl_Position = VP * vec4(worldPos, 1.0f);

	texCoord = vTexCoord;
	vNorm = vNormal;

	vec3 T = normalize(vec3(_model * vec4(vTangent, 0.0f)));
	//vec3 B = normalize(vec3(_model * vec4(vBiTangent, 0.0f)));
	vec3 N = normalize(vec3(_model * vec4(vNormal, 0.0f)));
	vec3 B = cross(N, T);

#ifdef IDKWHATTOCALLTHISBUTITISHERE
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	B = cross(N, T);
#endif 

	TBN = mat3(T, B, N);
}

#type fragment

#version 330 core
#extension ARB_bindless_texture : require

layout(location = 0) out vec3 pos; // 3 used
layout(location = 1) out vec3 diffuse_spec;
layout(location = 2) out vec3 normal; // 3 used
layout(location = 3) out vec2 specular; // 2 used

struct Material
{
	sampler2D diffuseMap;
	sampler2D normalsMap;
	sampler2D roughnessMap;
	sampler2D reflectMap;

	vec3 diffuse;
	float specular;
	float specularExponent;
};

layout(std140, binding = 6) uniform asd6
{
	Material materials[1000];
};
uniform int materialIdx;

// uniform struct MaterialTex
// {
// 	sampler2D _diffuse; // passed
// 	sampler2D _normals; // used
// 	sampler2D _roughness; // passed
// 	sampler2D _reflect; // set, not passed (metallic)
// } materialTex;

in vec3 worldPos;
in vec2 texCoord;
in mat3 TBN; // could do with lights from vert. shader (linear interpolation would help -> less matrix multiplications)(inverse -> transpose)
in vec3 vNorm;

// layout(bindless_sampler) uniform sampler2D diffuse; // passed
// layout(bindless_sampler) uniform sampler2D normals; // used
// layout(bindless_sampler) uniform sampler2D roughness; // passed
// layout(bindless_sampler) uniform sampler2D reflect; // set, not passed (metallic)

void main()
{
	pos = worldPos;
	diffuse_spec = texture(materials[materialIdx].diffuseMap, texCoord).rgb * materials[materialIdx].diffuse;
	specular = vec2(1 - texture(materials[materialIdx].roughnessMap, texCoord).r * materials[materialIdx].specular, materials[materialIdx].specularExponent);
	//diffuse_spec = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	vec3 norm = texture(materials[materialIdx].normalsMap, texCoord).rgb;
	norm = norm * 2.0f - 1.0f; // [0;1] -> [-1;1]
	normal = normalize(TBN * norm); // transforming from tangent-space -> world space

	// normal = texture(materialTex.normals, texCoord).rgb;
	// normal = normalize(vNorm);
}