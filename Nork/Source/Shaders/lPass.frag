#version 450

layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput gPos;
layout(input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput gCol;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 pos = subpassLoad(gPos);
	vec4 color = subpassLoad(gCol);
  outColor = color; //vec4(1.0);
}