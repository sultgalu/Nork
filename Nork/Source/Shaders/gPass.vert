#version 450

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

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec3 vTangent;

layout(location = 0) out vec3 worldPos;
layout(location = 1) out vec2 texCoord;
layout(location = 2) out mat3 TBN;
layout(location = 5) flat out Material material;

layout(push_constant) uniform constants
{
	mat4 VP;
} PushConstants;

layout(set = 0, binding = 1) readonly buffer asd5
{
	mat4 models[10];
}; 
layout(set = 0, binding = 2) readonly buffer asd6
{
	Material materials[10];
};
layout(set = 0, binding = 0) uniform asd8
{
	uvec4 modelMatIndexes[10]; // uvec2 and uvec4 have the same stride, so using the latter is more memory efficient, just a little more complicated in indexing
};

void main()
{
	uint drawIdx = gl_InstanceIndex; // gl_BaseInstance + gl_InstanceID;
	uint modelIdx = modelMatIndexes[drawIdx / 2][(drawIdx % 2) * 2];
	uint matIdx = modelMatIndexes[drawIdx / 2][(drawIdx % 2) * 2 + 1];
	material = materials[matIdx];
	mat4 _model = models[modelIdx];

	worldPos = (_model * vec4(vPos, 1.0f)).xyz;
	gl_Position = PushConstants.VP * vec4(worldPos, 1.0f);

	texCoord = vTexCoord;
	vec3 T = normalize(vec3(_model * vec4(vTangent, 0.0f)));
	vec3 N = normalize(vec3(_model * vec4(vNormal, 0.0f)));
	vec3 B = cross(N, T);

#ifdef REORTHOGONALIZE
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	B = cross(N, T);
#endif 

	TBN = mat3(T, B, N);
}