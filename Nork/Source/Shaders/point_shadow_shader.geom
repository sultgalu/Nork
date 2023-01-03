#version 450
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(location = 0) out vec4 worldPos;

struct VPS{
  mat4[6] VP;
};

layout(std140, set = 1, binding = 0) uniform u1
{
	VPS vpss[10];
};

layout(push_constant) uniform constants
{
  uint vpsIdx;
} PushConstants;

void main()
{
  VPS vps = vpss[PushConstants.vpsIdx];

	for (int face = 0; face < 6; face++)
	{
		gl_Layer = face;
		for (int i = 0; i < 3; i++)
		{
      mat4 vp = vps.VP[face];
			worldPos = gl_in[i].gl_Position;
			vec4 pos = vp * worldPos;
			gl_Position = pos;
			EmitVertex();
		}
		EndPrimitive();
	}
}