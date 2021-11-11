#type compute

#version 430 core
#extension GL_ARB_compute_shader : require
// group.x -> aabb1 (x)
// group.y -> aabb2 (y)

layout(local_size_x = 32) in;

struct Shape
{
	uint vertStart, vertCount;
	uint edgeStart, edgeCount;
	uint faceStart, faceCount; // faceHeaders
};
struct Face
{
	vec3 norm;
	uint vertIdx;
};
struct Result
{
	vec3 dir;
	float depth;
};
layout(std430, binding = 0) buffer INPUT0
{
	Shape shapes[];
};
layout(std140, binding = 1) buffer INPUT1
{
	mat4 models[];
};
layout(std140, binding = 3) buffer INPUT3
{
	vec3 verts[];
};
layout(std140, binding = 4) buffer INPUT4
{
	Face faces[]; // WILL NEED TO ROTATE
};
layout(std430, binding = 6) buffer inputASD
{
	Result filteredRes[]; // initial: -1
};

shared Result sharedResults[32];
shared uint largestIdx;

float signedDistance(vec3 dir, vec3 from, vec3 to);
vec3 farthest(uint start, uint count, vec3 dir);

void main()
{
	if (gl_LocalInvocationID.x == 0)
	{
		largestIdx = 0;
		sharedResults[0].depth = -10000000;
	}

	barrier();
	memoryBarrierShared();

	if (gl_WorkGroupID.y >= gl_WorkGroupID.x)
	{
		return;
	}

	Shape shape1 = shapes[gl_WorkGroupID.y]; // faces
	Shape shape2 = shapes[gl_WorkGroupID.x]; // verts

	uint localFaceIdx = gl_LocalInvocationID.x;
	if (localFaceIdx >= shape1.faceCount)
	{
		localFaceIdx -= shape1.faceCount;

		shape1 = shapes[gl_WorkGroupID.x]; // faces
		shape2 = shapes[gl_WorkGroupID.y]; // verts

		if (localFaceIdx >= shape1.faceCount)
			return;
	}

	uint globalFaceIdx = shape1.faceStart + localFaceIdx;
	faces[globalFaceIdx].norm = normalize(faces[globalFaceIdx].norm);
	Face face = faces[globalFaceIdx];

	vec3 farthestVertTowardsFace = farthest(shape2.vertStart, shape2.vertCount, -face.norm);

	vec3 somePointOnFace = verts[face.vertIdx];
	float pointDistanceFromFaceOutwards = signedDistance(face.norm, somePointOnFace, farthestVertTowardsFace);

	uint ourIdx = gl_LocalInvocationID.x;
	sharedResults[ourIdx].depth = pointDistanceFromFaceOutwards; // float(verts[gl_WorkGroupID.x + gl_LocalInvocationID.x].y);;
	sharedResults[ourIdx].dir = face.norm;

	uint idx = largestIdx;
	while (sharedResults[idx].depth < sharedResults[ourIdx].depth)
	{
		atomicCompSwap(largestIdx, idx, ourIdx);
		idx = largestIdx;
	}

	barrier();
	memoryBarrierShared();

	if (gl_LocalInvocationID.x == 0)
	{
		sharedResults[largestIdx].dir *= -1;
		sharedResults[largestIdx].depth *= -1; // correction for resolution phase, should handle better
		filteredRes[gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x] = sharedResults[largestIdx];
		//filteredRes[gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x].depth = float(ourIdx);
		//filteredRes[gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x].dir = vec3(gl_WorkGroupID.y, gl_WorkGroupID.x, 69);
	}
}





float signedDistance(vec3 dir, vec3 from, vec3 to)
{
	vec3 vec = to - from;
	return dot(normalize(dir), vec);
}
vec3 farthest(uint start, uint count, vec3 dir)
{
	uint farthest = start;
	float largestDot = dot(dir, verts[start]);

	for (uint i = start + 1; i < start + count; i++)
	{
		float dot = dot(dir, verts[i]);
		if (dot > largestDot)
		{
			largestDot = dot;
			farthest = i;
		}
	}

	return verts[farthest];
}