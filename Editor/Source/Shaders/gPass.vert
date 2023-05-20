#version 450

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

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec3 vTangent;
layout(location = 4) in uint modelIdx;
layout(location = 5) in uint matIdx;

layout(location = 0) out vec3 worldPos;
layout(location = 1) out vec2 texCoord;
layout(location = 2) out mat3 TBN;
layout(location = 5) flat out Material material;

layout(push_constant) uniform constants
{
	mat4 VP;
} PushConstants;

layout(set = 0, binding = 0) readonly buffer asd5
{
	mat4 models[10];
}; 
layout(set = 0, binding = 1) readonly buffer asd6
{
	Material materials[10];
};

void main()
{
	material = materials[matIdx];
	mat4 _model = models[modelIdx];

	worldPos = (_model * vec4(vPos, 1.0f)).xyz;
	gl_Position = PushConstants.VP * vec4(worldPos, 1.0f);
	texCoord = vTexCoord;

	vec3 T = normalize(vec3(_model * vec4(vTangent, 0.0)));
	vec3 N = normalize(vec3(_model * vec4(vNormal, 0.0)));
	vec3 B = cross(N, T);
#ifdef REORTHOGONALIZE
	T = normalize(T - dot(T, N) * N);// re-orthogonalize T with respect to N
	B = cross(N, T);// then retrieve perpendicular vector B with the cross product of T and N
#endif 
	TBN = mat3(T, B, N);
}