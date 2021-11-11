#type compute

#version 330 core
#extension GL_ARB_compute_shader : require
// group.x -> aabb1 (x)
// group.y -> aabb2 (y)

layout(local_size_x = 1) in;
struct AABB
{
	float[3] min;
	float[3] max;
};
layout(std430, binding = 0) buffer inputAABBS
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
	for (uint i = 0; i < 3; i++)
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
	res[gl_WorkGroupID.x * gl_NumWorkGroups.x + gl_WorkGroupID.y] = count;
	//res[gl_WorkGroupID.x * gl_NumWorkGroups.x + gl_WorkGroupID.y] = uint(aabb1.max[1]);
}