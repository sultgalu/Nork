#type vertex
#version 330 core
layout(location = 0) in vec3 vPos;

uniform mat4 model;

void main()
{
	gl_Position = model * vec4(vPos, 1.0f);
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