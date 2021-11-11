#type compute

#version 430 core
#extension GL_ARB_compute_shader : require
// group.x -> aabb1 (x)
// group.y -> aabb2 (y)

layout(local_size_x = 1024) in;

struct Kinematic
{
	vec3 pos;
	bool isStatic;
	vec3 velocity;
	float mass;
};
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
struct SatResult
{
	vec3 dir;
	float depth;
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
	uint aabbRes[];
};
layout(std430, binding = 6) buffer inputASD
{
	SatResult satRes[]; // initial: -1
};
layout(std140, binding = 7) buffer INPUT7
{
	vec3 centers[];
};
layout(std430, binding = 8) buffer INPUT8
{
	uvec2 edges[];
};
layout(std430, binding = 8) buffer INPUT8
{
	uvec2 edges[];
};
layout(std430, binding = 10) buffer INPUT10
{
	Kinematic kinems[];
};

shared uint counter;

void main()
{
	queue[gl_LocalInvocationID.x] = 0;

	barrier();
	memoryBarrierShared();

	uint idx1 = gl_WorkGroupID.x;
	uint idx2 = gl_LocalInvocationID.x;

	uint satIdx;
	if (gl_WorkGroupID.x < gl_LocalInvocationID.x)
		satIdx = gl_WorkGroupID.x * gl_NumWorkGroups.x + gl_LocalInvocationID.x;
	else if (gl_WorkGroupID.x > gl_LocalInvocationID.x)
		satIdx = gl_LocalInvocationID.x * gl_NumWorkGroups.x + gl_WorkGroupID.x;
	else return; // don't resolve with self

	// ----------------------RESOLUTION--------------------------

	SatResult coll = satRes[satIdx];

	if (coll.depth < -0.0001f)
		return;
	
}