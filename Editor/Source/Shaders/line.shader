#type vertex

#version 330 core

layout(location = 0) in vec3 vPos;

uniform mat4 VP;

void main()
{
	gl_Position = VP * vec4(vPos, 1.0f);
}

#type geometry
#version 330 core
layout(lines) in;
layout(triangle_strip, max_vertices = 6) out;

uniform float width;

void main()
{
	vec2 p1 = gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w; // bottom
	vec2 p2 = gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w; // top

	float depth1 = gl_in[0].gl_Position.z / gl_in[0].gl_Position.w;
	float depth2 = gl_in[1].gl_Position.z / gl_in[1].gl_Position.w;

	vec2 dir = p1 - p2;
	vec2 norm1 = normalize(vec2(dir.y, -dir.x)); // left
	vec2 norm2 = normalize(vec2(-dir.y, dir.x)); // right

	vec3 vert0 = vec3(p1 + norm1 * width, depth1); // bottom-left
	vec3 vert1 = vec3(p1 + norm2 * width, depth1); // bottom-right
	vec3 vert2 = vec3(p2 + norm2 * width, depth2); // top-right
	vec3 vert3 = vec3(p2 + norm1 * width, depth2); // top-left

	gl_Position = vec4(vert0.xyz, 1);
	EmitVertex();
	gl_Position = vec4(vert1.xyz, 1);
	EmitVertex();
	gl_Position = vec4(vert3.xyz, 1);
	EmitVertex();
	EndPrimitive();

	gl_Position = vec4(vert3.xyz, 1);
	EmitVertex();
	gl_Position = vec4(vert1.xyz, 1);
	EmitVertex();
	gl_Position = vec4(vert2.xyz, 1);
	EmitVertex();
	EndPrimitive();
}

#type fragment
#version 330 core

out vec4 fColor;

uniform vec4 colorDefault;
uniform vec4 colorSelected;

uniform int selected = 0;

void main()
{
	fColor = selected * colorSelected + (1.0f - selected) * colorDefault;
	if (selected > 0)
	{
		fColor = colorSelected;
	}
	//fColor = colorDefault;
}