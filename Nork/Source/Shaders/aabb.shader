#type compute

#version 430 core
#extension GL_ARB_compute_shader : require

layout(local_size_x = 1024) in;
//layout(local_size_x = 1) in;

struct Shape
{
	uint vertStart, vertCount;
	uint edgeStart, edgeCount;
	uint faceStart, faceCount;
};
struct AABB
{
	vec3 min;
	vec3 max;
};
layout(std430, binding = 0) buffer INPUT0
{
	Shape shapes[];
};
layout(std140, binding = 1) buffer INPUT1
{
	mat4 models[];
};
layout(std430, binding = 2) buffer INPUT2
{
	AABB aabbs[];
};
layout(std140, binding = 3) buffer INPUT3
{
	vec3 verts[];
};
layout(std430, binding = 5) buffer INPUT5
{
	uvec2 aabbRes[];
};
layout(binding = 0, offset = 0) uniform atomic_uint counter;

void main()
{
	uint idx1 = gl_WorkGroupID.y * 1024 + gl_LocalInvocationID.x;
	uint idx2 = gl_WorkGroupID.x;

	if (idx1 >= idx2)
	{
		return;
	}

	AABB aabb1 = aabbs[idx1];
	AABB aabb2 = aabbs[idx2];

	for (uint i = 0; i < 3; i++)
	{
		if (aabb1.min[i] < aabb2.min[i])
		{
			if (aabb1.max[i] > aabb2.min[i])
				continue;
		}
		else
		{
			if (aabb1.min[i] < aabb2.max[i])
				continue;
		}
		return;
	}
	uint idx = atomicCounterIncrement(counter);
	aabbRes[idx] = uvec2(idx1, idx2);
}