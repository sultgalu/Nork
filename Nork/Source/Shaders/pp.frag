#version 450

layout(input_attachment_index = 4, set = 1, binding = 4) uniform subpassInput frame;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 color = subpassLoad(frame).rgb;
  // outColor = color; //.grba;
  // outColor = vec4(1.0);
  float exposure = 10.0f;
	color = vec3(1.0f) - exp(-color * exposure);
  // float gamma = 0.1;
	// color = pow(color, vec3(1.0f / gamma));
  // gamma correction happens when you render / copy to sRGB image (from linear to gamma space)
  outColor.rgb = color.rgb;
  outColor.a = 1.0f;
}