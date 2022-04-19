#type vertex
#version 330 core
layout(location = 0) in vec3 vPos;
#extension ARB_shader_draw_parameters : require

layout(std140, binding = 5) uniform asd5
{
	mat4 models[1];
};
layout(std140, binding = 7) uniform asd8
{
	uvec4 modelMatIndexes[1];
};

void main()
{
	uint drawIdx = gl_BaseInstance + gl_InstanceID;
	uint modelIdx = modelMatIndexes[drawIdx / 2][(drawIdx % 2) * 2];
	gl_Position = models[modelIdx] * vec4(vPos, 1.0f);
}

#type geometry
#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4[6] VP;
out vec4 worldPos;

void main()
{
	for (int face = 0; face < 6; face++)
	{
		gl_Layer = face;
		for (int i = 0; i < 3; i++)
		{
			worldPos = gl_in[i].gl_Position;
			gl_Position = VP[face] * worldPos;
			EmitVertex();
		}
		EndPrimitive();
	}
}

#type fragment
#version 330 core
	
in vec4 worldPos;

uniform float far;
uniform vec3 ligthPos;

void main()
{
	float distance = length(worldPos.xyz - ligthPos);
	gl_FragDepth = distance / far; // to fit in [0;1] range
}