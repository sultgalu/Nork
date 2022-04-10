#type vertex
#version 330 core

layout(location = 0) in vec3 vPos;
//layout(location = 1) in float isSelected;

uniform mat4 VP;
//out float selected;

void main()
{
	gl_Position = VP * vec4(vPos, 1.0f);
	//selected = isSelected;
}

#type fragment
#version 330 core

out vec4 fColor;
uniform vec4 colorDefault;
uniform vec4 colorSelected;
//in float selected;

void main()
{
	//int sel = int(selected);
	//fColor = sel * colorSelected + (1 - sel) * colorDefault;
	fColor = colorDefault;
}