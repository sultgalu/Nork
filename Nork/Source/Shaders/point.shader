#type vertex

#version 330 core

layout(location = 0) in vec3 vPos;
// layout(location = 1) in float isSelected;
// layout(location = 2) in uint vId;

uniform mat4 VP;
// out float selected;
// flat out uint fId;

void main()
{
	gl_Position = VP * vec4(vPos, 1.0f);
	// selected = isSelected;
	// fId = vId;
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color; // 3 used
// layout(location = 1) out uint id;

// in float selected;
// flat in uint fId;

uniform float aa; // anti-aliasing
uniform vec4 colorDefault;
uniform vec4 colorSelected;
uniform float size;

void main()
{
	// vec4 fColor = selected * colorSelected + (1 - selected) * colorDefault;
	vec4 fColor = colorDefault;
	float distance = length(gl_PointCoord * 2 - 1); // [0,1]->[-1;1]
	if (distance < size)
	{
		color = fColor;
	}
	else if (distance < size + aa)
	{
		float alpha = (size + aa - distance) / aa;
		color = vec4(fColor.rgb, clamp(fColor.a - 1 + alpha, 0.0f, 1.0f));
	}
	else
	{
		color = vec4(0);
	}
	// id = fId;
	// color = vec4(1.0f);
}