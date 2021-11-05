#type compute

#version 330 core

// group.x -> aabb1 (x)
// group.y -> aabb2 (y)

layout(local_size_x = 3​) in;

struct AABB
{
	vec3 min;
	vec3 max;
};

layout(std430, binding = 0) buffer input
{
	AABB aabbs[];
};

layout(std430, binding = 1) buffer results
{
	uint res[];
};


void main()
{
	AABB aabb1 = aabbs[gl_WorkGroupID.x];
	AABB aabb2 = aabbs[gl_WorkGroupID.y];

	int count = 0;
	for (size_t i = 0; i < 3; i++)
	{
		if (aabb1.min[i] < aabb2.min[i])
		{
			if (aabb1.max[i] > aabb2.min[i])
				count++;
		}
		else
		{
			if (aabb1.min[i] < aabb2.max[i])
				count++;
		}
	}

	res[gl_WorkGroupID.x * gl_NumWorkGroups.x + gl_WorkGroupID.y] = count == 3 ? 1 : 0;
}