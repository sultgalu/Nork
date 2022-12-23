#version 450

layout(input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput frame;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 color = subpassLoad(frame);
  outColor = color.grba; //vec4(1.0);
}