#type vertex

#version 330 core
#extension ARB_bindless_texture : require
#extension ARB_shader_draw_parameters : require

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec3 vTangent;

out vec3 worldPos;
out vec2 texCoord;
out vec4 lightViewPos; // new
out mat3 TBN;
out vec3 vNorm;

uniform mat4 model;
uniform mat4 VP;

layout(std140, binding = 5) uniform asd5
{
	mat4 models[1];
}; 
layout(std140, binding = 8) uniform asd8
{
	uvec4 modelIndexes[1];
};

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
	Material materials[1];
};
layout(std140, binding = 7) uniform asd7
{
	uvec4 materialIndices[1];
};
flat out Material material;

void main()
{
	uint drawIdx = gl_BaseInstance + gl_InstanceID;
	uint matIdx = materialIndices[drawIdx / 4][drawIdx % 4]; // needed because of int[] padding (pads it up to a vec4)
	uint modelIdx = modelIndexes[drawIdx / 4][drawIdx % 4];
	material = materials[matIdx];
	mat4 _model = models[modelIdx];

	worldPos = (_model * vec4(vPos, 1.0f)).xyz;
	gl_Position = VP * vec4(worldPos, 1.0f);

	texCoord = vTexCoord;
	vNorm = vNormal;

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

#type fragment

#version 330 core
#extension ARB_bindless_texture : require
//#extension ARB_shader_draw_parameters : require

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

flat in Material material;
in vec3 worldPos;
in vec2 texCoord;
in mat3 TBN; // could do with lights from vert. shader (linear interpolation would help -> less matrix multiplications)(inverse -> transpose)
in vec3 vNorm;

// layout(std140, binding = 6) uniform asd6
// {
// 	Material materials[1];
// };
// 
// layout(std140, binding = 7) uniform asd7
// {
// 	uint materialIndices[1];
// };
void main()
{
	//Material material = materials[0]; // problme is here
	
	pos = worldPos;
	diffuse_spec = texture(material.diffuseMap, texCoord).rgb * material.diffuse;
	specular = vec2(1 - texture(material.roughnessMap, texCoord).r * material.specular, material.specularExponent);
	
	vec3 norm = texture(material.normalsMap, texCoord).rgb;
	norm = norm * 2.0f - 1.0f; // [0;1] -> [-1;1]
	normal = normalize(TBN * norm); // transforming from tangent-space -> world space
}