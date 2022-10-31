#type vertex

#version 430 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in uint vId;
// layout(location = 1) in float isSelected;

flat out uint vertId;
uniform mat4 VP;
// out float selected;
// flat out uint fId;

void main()
{
	gl_Position = VP * vec4(vPos, 1.0f);
	vertId = vId;
	// selected = isSelected;
	// fId = vId;
}

#type fragment
#version 430 core

layout(location = 0) out vec4 color; // 3 used
// layout(location = 1) out uint id;

// in float selected;
// flat in uint fId;

flat in uint vertId;

uniform float aa; // anti-aliasing
uniform vec4 colorDefault;
uniform vec4 colorSelected;
uniform float size;
uniform float bias;
uniform int selected = 0;

layout(std140, binding = 10) buffer asd0
{
	uint x, y;
	uint id;
};

void main()
{
	vec4 fColor = colorDefault;
	//if (y > 50)
	// id = 2;
	if (selected == 0 && abs(gl_FragCoord.x - x) < 1 && abs(gl_FragCoord.y - y) < 1)
	{
		id = vertId;
		// fColor = colorSelected;
	}
	if (selected > 0)
	{
		fColor = colorSelected;
	}
	// vec4 fColor = selected * colorSelected + (1 - selected) * colorDefault;
	float distance = length(gl_PointCoord * 2 - 1); // [0,1]->[-1;1]
	if (distance < size)
	{
		color = fColor;
	}
	else if (size + aa - distance > bias)
	{
		float alpha = (size + aa - distance) / aa;
		color = vec4(fColor.rgb, clamp(fColor.a - 1 + alpha, 0.0f, 1.0f)); 
	}
	else
	{
		discard;
	}
	gl_FragDepth = gl_FragCoord.z - 0.001f; // push points a little in front of line engings
	// id = fId;
	// color = vec4(1.0f);
}