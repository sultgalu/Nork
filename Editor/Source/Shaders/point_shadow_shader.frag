#version 450
	
layout(location = 0) in vec4 worldPos;

layout(push_constant) uniform constants
{
  layout(offset = 4) float far;
  layout(offset = 8) float padding;
  layout(offset = 16) vec3 ligthPos;
} PushConstants;


void main()
{
	float distance = length(worldPos.xyz - PushConstants.ligthPos);
	gl_FragDepth = distance / PushConstants.far; // to fit in [0;1] range
}