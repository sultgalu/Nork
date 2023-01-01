#version 450

layout(input_attachment_index = 4, set = 1, binding = 4) uniform subpassInput frame;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 color = subpassLoad(frame).rgb;
  // outColor = color; //.grba;
  // outColor = vec4(1.0);
  float gamma = 1.0f;
  float exposure = 10.0f;
	color = vec3(1.0f) - exp(-color * exposure);
	outColor.rgb = pow(color, vec3(1.0f / gamma));
  outColor.a = 1.0f;
}