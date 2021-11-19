#type compute

#version 430 core
#extension GL_ARB_compute_shader : require
#extension GL_ARB_shader_atomic_counter_ops : require

layout(local_size_x = 1024) in;

struct PointLight
{
	vec3 position;
	vec4 color;

	float linear, quadratic;
};

struct Config
{
	mat4 V;
	mat4 iP;
	uint lightCount;
	uint atomicCounter;
	uvec2 cullRes;
};

layout(std430, binding = 20) buffer BUF1
{
	uint lightIndicies[];
};
layout(std430, binding = 21) buffer BUF2
{
	uvec2 ranges[];
};
layout(std430, binding = 22) buffer BUF3
{
	Config config;
};
layout(std140, binding = 3) uniform asd3
{
	PointLight lights[1024];
};
struct Rect
{
	vec2 ur;
	vec2 lr;
	vec2 ll;
	vec2 ul;
	vec2 center;
};

shared uint sharedIndices[1024];
shared uint count;

struct Plane
{
	vec3 norm;
	vec3 point;
};

#define _0_0 0
#define _1_0 1
#define _1_1 2
#define _0_1 3

struct Frustum
{
	vec3 nears[4];
	vec3 fars[4];
};

Plane getPlane(Frustum fr, uint idx1, uint idx2, uint idxForNormCheck)
{
	vec3 edge1 = fr.fars[idx1] - fr.nears[idx1];
	vec3 edge2 = fr.fars[idx1] - fr.nears[idx2];
	
	Plane plane;
	plane.norm = normalize(cross(edge2, edge1));
	plane.point = fr.fars[idx1];

	if (dot(plane.norm, plane.point - fr.fars[idxForNormCheck]) < 0) // if normal points inside, correct it
	{
		plane.norm *= -1;
	}
	return plane;
}

bool isLightReaching(Plane plane, vec3 lightPos, float distance)
{
	vec3 planeToLightVec = lightPos - plane.point;
	float sdFromPlane = dot(planeToLightVec, plane.norm); // +(outside) -(inside)
	float lightReachDistance = sdFromPlane - distance;
	return lightReachDistance < 0;
}

vec3 screenToView(vec3 pos)
{
	vec4 temp = config.iP * vec4(pos.x * 2 - 1.0f, pos.y * 2 - 1.0f, pos.z, 1);
	return temp.xyz / temp.w;
}

Frustum getFrustum()
{
	vec2 fragSize = vec2(1 / float(gl_NumWorkGroups.x), 1 / float(gl_NumWorkGroups.y));
	vec2 fragCoord = vec2(gl_WorkGroupID.x * fragSize.x, gl_WorkGroupID.y * fragSize.y);

	Frustum fr;
	fr.nears[_0_0] = vec3(fragCoord.x,				fragCoord.y,				0);
	fr.nears[_1_0] = vec3(fragCoord.x + fragSize.x, fragCoord.y,				0);
	fr.nears[_1_1] = vec3(fragCoord.x + fragSize.x, fragCoord.y + fragSize.y,	0);
	fr.nears[_0_1] = vec3(fragCoord.x,				fragCoord.y + fragSize.y,	0);

	fr.fars[_0_0] = vec3(fragCoord.x,				fragCoord.y,				1);
	fr.fars[_1_0] = vec3(fragCoord.x + fragSize.x,	fragCoord.y,				1);
	fr.fars[_1_1] = vec3(fragCoord.x + fragSize.x,	fragCoord.y + fragSize.y,	1);
	fr.fars[_0_1] = vec3(fragCoord.x,				fragCoord.y + fragSize.y,	1);
	
	for (uint i = 0; i < 4; i++)
		fr.nears[i] = screenToView(fr.nears[i]);
	for (uint i = 0; i < 4; i++)
		fr.fars[i] = screenToView(fr.fars[i]);

	return fr;
}

void main()
{
	if (gl_LocalInvocationID.x >= config.lightCount)
		return;

	// shared data setup
	if (gl_LocalInvocationID.x == 0)
	{
		count = 0;
	}

	barrier();
	memoryBarrierShared();

	// local job
	
	PointLight light = lights[gl_LocalInvocationID.x];
	vec3 lightViewPos = (config.V * vec4(light.position, 1)).xyz;

	float linear = light.linear;
	float quadratic = light.quadratic;
	float accuracy = 1000.0f;
	float distance = -linear + sqrt(linear * linear + 4 * quadratic * accuracy) / (2 * quadratic);

	// should be shared?
	Frustum fr = getFrustum();
	if (isLightReaching(getPlane(fr, _0_0, _0_1, _1_1), lightViewPos, distance) &&
		isLightReaching(getPlane(fr, _0_1, _1_1, _1_0), lightViewPos, distance) &&
		isLightReaching(getPlane(fr, _1_1, _1_0, _0_0), lightViewPos, distance) &&
		isLightReaching(getPlane(fr, _1_0, _0_0, _0_1), lightViewPos, distance))
	{
		uint idx = atomicAdd(count, 1);
		sharedIndices[idx] = gl_LocalInvocationID.x;
	}
	// ----------------

	barrier();
	memoryBarrierShared();

	if (gl_LocalInvocationID.x == 0)
	{
		uint idxOffset = atomicAdd(config.atomicCounter, count);
		for (uint i = 0; i < count; i++)
		{
			lightIndicies[idxOffset + i] = sharedIndices[i];
		}
		ranges[gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x] = uvec2(idxOffset, count);
	}
}