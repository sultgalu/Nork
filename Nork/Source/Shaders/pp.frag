#version 450

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput frame;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 color = subpassLoad(frame);
  outColor = color.bgra; //vec4(1.0);
}