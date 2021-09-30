#type vertex

#version 330 core

layout(location = 0) in vec3 vPos;

out vec3 texCoords;

uniform mat4 VP;

void main()
{
	texCoords = vPos;
	gl_Position = VP * vec4(vPos, 1.0f);
	gl_Position = gl_Position.xyww;
}

#type fragment

#version 330 core

layout(location = 0) out vec4 fColor;

in vec3 texCoords;

uniform samplerCube skyBox;
uniform sampler2D gradient;
//uniform vec3 tint = vec3(1, 1, 1);
uniform vec3 texCoord;

void main()
{
	//vec3 grad = texture(gradient, texCoord.xy).rgb;
	//fColor = vec4(texture(skyBox, texCoords).rgb + tint.rgb, 1.0f);
	
	//fColor = texture(skyBox, texCoords) * vec4(grad.rgb, 0.0f);
	//fColor = vec4(grad.rgb, 1.0f);
	fColor = texture(skyBox, texCoords);
	//fColor = vec4(1.0f, 0.0f, 1.0f, 1.0f);
}