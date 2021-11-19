#type compute

#version 430 core
#extension GL_ARB_compute_shader : require
#extension GL_ARB_shader_atomic_counter_ops : require

layout(local_size_x = 256) in;

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
	PointLight lights[256];
};
//layout(std430, binding = 23) buffer BUF4
//{
//	PointLight lights[];
//};
//layout(binding = 0, offset = 0) uniform atomic_uint indexCounter;

struct Rect
{
	vec2 ur;
	vec2 lr;
	vec2 ll;
	vec2 ul;
	vec2 center;
};

shared uint sharedIndices[256];
shared uint count;
shared vec2 fragCoord;
shared vec3 centerNear;
shared vec3 centerVec;

bool face(vec3 planeNorm, vec3 planePoint, vec3 lightPos, float distance)
{
	vec3 planeToLightVec = lightPos - planePoint;
	float sdFromPlane = dot(planeToLightVec, planeNorm); // +(outside) -(inside)
	float lightReachDistance = distance - sdFromPlane;
	return lightReachDistance < 0;
}

void main()
{
	if (gl_LocalInvocationID.x >= config.lightCount)
		return;

	// shared data setup
	if (gl_LocalInvocationID.x == 0)
	{
		vec2 size = vec2(1 / float(gl_NumWorkGroups.x), 1 / float(gl_NumWorkGroups.y));

		count = 0;
		fragCoord = vec2(float(gl_WorkGroupID.x) / float(gl_NumWorkGroups.x), float(gl_WorkGroupID.y) / float(gl_NumWorkGroups.y));
		fragCoord += vec2(size.x / 2.0f, size.y / 2.0f);
		fragCoord.x = fragCoord.x * 2 - 1.0f;
		fragCoord.y = fragCoord.y * 2 - 1.0f;

		vec4 centerNear1 = (config.iP * vec4(fragCoord, 0, 1));
		centerNear = vec3(centerNear1) / centerNear1.w;

		vec4 centerFar1 = (config.iP * vec4(fragCoord, 1, 1));
		vec3 centerFar = vec3(centerFar1) / centerFar1.w;

		centerVec = centerFar - centerNear;
	}

	barrier();
	memoryBarrierShared();

	// local job
	
	PointLight light = lights[gl_LocalInvocationID.x];
	vec3 lightViewPos = (config.V * vec4(light.position, 1)).xyz;

	// linear * distance + quadratic * distance * distance - 1000 = 0
	// quadratic * distance * distance + linear * distance - 1000 = 0
	// a * x * x + b * x + c = 0 -> a = quadratic, b = linear, c = -accuracy
	// x = -b + sqrt(b*b - 4*a*c) / (2*a)
	float linear = light.linear;
	float quadratic = light.quadratic;
	float accuracy = 1000.0f;
	float distance = -linear + sqrt(linear * linear + 4 * quadratic * accuracy) / (2 * quadratic);

	vec3 plane = cross(centerVec, lightViewPos - centerNear);
	vec3 partLineToLightDir = cross(plane, centerVec);
	float actualDistance = dot(normalize(partLineToLightDir), lightViewPos - centerNear);

	if (distance > actualDistance)
	{
		uint idx = atomicAdd(count, 1);
		sharedIndices[idx] = gl_LocalInvocationID.x;
	}

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