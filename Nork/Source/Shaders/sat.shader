#type compute

#version 430 core
#extension GL_ARB_compute_shader : require
// group.x -> aabb1 (x)
// group.y -> aabb2 (y)

layout(local_size_x = 1024) in;

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
	uvec2 pair;
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
layout(std430, binding = 4) buffer INPUT4
{
	Face faces[]; // WILL NEED TO ROTATE
};
layout(std430, binding = 5) buffer INPUT5
{
	uvec2 aabbRes[];
};
layout(std430, binding = 6) buffer inputASD
{
	Result satRes[]; // initial: -1
};
layout(std140, binding = 7) buffer INPUT7
{
	vec3 centers[];
};
layout(std430, binding = 8) buffer INPUT8
{
	uvec2 edges[];
};
layout(binding = 0, offset = 0) uniform atomic_uint aabbCounter;
layout(binding = 0, offset = 4) uniform atomic_uint counter;

shared Result sharedResults[1024];
shared uint largestIdx;

float signedDistance(vec3 dir, vec3 from, vec3 to);
vec3 farthest(uint start, uint count, vec3 dir);

Result facePhase(Shape shape1, Shape shape2, int dirMultiplier, uint faceIdx);
Result edgePhase(Shape shape1, Shape shape2, vec3 center1, uint i, uint j);

void main()
{
	if (gl_LocalInvocationID.x == 0)
	{
		largestIdx = 0;
		sharedResults[0].depth = -10000000;
	}

	barrier();
	memoryBarrierShared();

	uvec2 aabbIdx = aabbRes[gl_WorkGroupID.x];
	Shape shape1 = shapes[aabbIdx.x]; // faces
	Shape shape2 = shapes[aabbIdx.y]; // verts

	Result threadRes;

	if (gl_LocalInvocationID.x < shape1.faceCount)
	{
		uint faceIdx = shape1.faceStart + gl_LocalInvocationID.x;
		threadRes = facePhase(shape1, shape2, -1, faceIdx);
	}
	else if (gl_LocalInvocationID.x < shape1.faceCount + shape2.faceCount)
	{
		uint faceIdx = shape2.faceStart + gl_LocalInvocationID.x - shape1.faceCount;
		threadRes = facePhase(shape2, shape1, 1, faceIdx);
	}
	else if (gl_LocalInvocationID.x < shape1.faceCount + shape2.faceCount + shape1.edgeCount * shape2.edgeCount)
	{
		uint edgeInvIdx = gl_LocalInvocationID.x - shape1.faceCount - shape2.faceCount; // - shape1.faceCount + shape2.faceCount;
		uint edgeIdx1 = edgeInvIdx / shape2.edgeCount;
		uint edgeIdx2 = edgeInvIdx % shape2.edgeCount;
		threadRes = edgePhase(shape1, shape2, centers[gl_WorkGroupID.y], edgeIdx1, edgeIdx2);
	}
	else
	{
		return;
	}

	uint ourIdx = gl_LocalInvocationID.x;
	sharedResults[ourIdx] = threadRes;

	uint idx = largestIdx;
	while (sharedResults[idx].depth < threadRes.depth)
	{
		atomicCompSwap(largestIdx, idx, ourIdx);
		idx = largestIdx;
	}

	barrier();
	memoryBarrierShared();

	if (gl_LocalInvocationID.x == 0)
	{
		if (sharedResults[largestIdx].depth <= 0)
		{
			uint idx = atomicCounterIncrement(counter);
			sharedResults[largestIdx].pair = uvec2(aabbIdx.x, aabbIdx.y);
			sharedResults[largestIdx].depth *= -1; // correction for resolution phase, should handle better
			satRes[idx] = sharedResults[largestIdx];
		}
	}
}

Result facePhase(Shape shape1, Shape shape2, int dirMultiplier, uint faceIdx)
{
	faces[faceIdx].norm = normalize(faces[faceIdx].norm);
	Face face = faces[faceIdx];

	vec3 farthestVertTowardsFace = farthest(shape2.vertStart, shape2.vertCount, -face.norm);

	vec3 somePointOnFace = verts[face.vertIdx];
	float pointDistanceFromFaceOutwards = signedDistance(face.norm, somePointOnFace, farthestVertTowardsFace);

	Result result;
	result.depth = pointDistanceFromFaceOutwards; // float(verts[gl_WorkGroupID.x + gl_LocalInvocationID.x].y);;
	result.dir = face.norm * dirMultiplier;

	return result;
}

Result edgePhase(Shape shape1, Shape shape2, vec3 center1, uint i, uint j)
{
	uvec2 edge1 = edges[shape1.edgeStart + i];
	uvec2 edge2 = edges[shape2.edgeStart + j];

	vec3 edgeDir1 = verts[edge1.x] - verts[edge1.y];
	vec3 edgeDir2 = verts[edge2.x] - verts[edge2.y];

	vec3 edge1ToEdge2Normal = cross(edgeDir2, edgeDir1);

	if (dot(edge1ToEdge2Normal, edge1ToEdge2Normal) == 0)
		return Result(vec3(0, 0, 0), -100000, uvec2(0, 0)); // calculating with 0;0;0 normal doesn't make sense

	if (signedDistance(edge1ToEdge2Normal, verts[edge1.x], center1) > 0)
		edge1ToEdge2Normal *= -1; // edge normal faces inwards, correct it

	vec3 closest2 = farthest(shape2.vertStart, shape2.vertCount, -edge1ToEdge2Normal); // Get the closest point towards c1
	vec3 closest1 = farthest(shape1.vertStart, shape1.vertCount, edge1ToEdge2Normal); // Get the closest point fromwards c1
	float distance = signedDistance(edge1ToEdge2Normal, closest1, closest2);

	Result result;
	result.depth = distance;
	result.dir = -normalize(edge1ToEdge2Normal);

	return result;
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