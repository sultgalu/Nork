#type compute

#version 430 core
#extension GL_ARB_compute_shader : require

layout(local_size_x = 32) in;

struct Shape
{
	uint vertStart, vertCount;
	uint edgeStart, edgeCount;
	uint faceStart, faceCount;
};
struct Face
{
	vec3 norm;
	uint vertIdx;
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
layout(std430, binding = 1) buffer INPUT1
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
layout(std430, binding = 4) buffer INPUT4
{
	Face faces[]; // WILL NEED TO ROTATE
};
layout(std140, binding = 7) buffer INPUT7
{
	vec3 centers[];
};
layout(std430, binding = 8) buffer INPUT8
{
	uvec2 edges[];
};
layout(std430, binding = 9) buffer INPUT9
{
	vec3 vertsIn[];
};

shared uvec3 min;
shared uvec3 max;

void main()
{
	Shape shape = shapes[gl_WorkGroupID.x];
	if (gl_LocalInvocationID.x >= shape.vertCount)
	{
		if (gl_LocalInvocationID.x == shape.vertCount)
		{
			min = uvec3(shape.vertStart);
			max = uvec3(shape.vertStart);
		}
		barrier();
		memoryBarrierShared();
		return;
	}

	uint localVertIdx = shape.vertStart + gl_LocalInvocationID.x;
	vec3 transformedVert = (models[gl_WorkGroupID.x] * vec4(vertsIn[localVertIdx], 1)).xyz;
	verts[localVertIdx] = transformedVert;
	
	 //------------ AABBS ------------------
	barrier();
	memoryBarrierShared();
	groupMemoryBarrier();

	for (uint axis = 0; axis < 3; axis++)
	{
		uint current = min[axis];
		while (verts[current][axis] > verts[localVertIdx][axis])
		{
			atomicCompSwap(min[axis], current, localVertIdx);

			current = min[axis];
		}
	}

	for (uint axis = 0; axis < 3; axis++)
	{
		uint current = max[axis];
		while (verts[current][axis] < verts[localVertIdx][axis])
		{
			atomicCompSwap(max[axis], current, localVertIdx);

			current = max[axis];
		}
	}
;
	barrier();
	memoryBarrierShared();

	if (gl_LocalInvocationID.x < shape.faceCount)
	{
		uint faceIdx = shape.faceStart + gl_LocalInvocationID.x;
		faces[faceIdx].norm = normalize(faces[faceIdx].norm);
	}
	if (gl_LocalInvocationID.x == 0)
	{
		aabbs[gl_WorkGroupID.x].min = vec3(verts[min.x].x, verts[min.y].y, verts[min.z].z);
		aabbs[gl_WorkGroupID.x].max = vec3(verts[max.x].x, verts[max.y].y, verts[max.z].z);
		vec3 center;
		for (uint i = 0; i < shape.vertCount; i++)
		{
			center += verts[shape.vertStart + i];
		}
		centers[gl_WorkGroupID.x] = center / shape.vertCount;
	}
}