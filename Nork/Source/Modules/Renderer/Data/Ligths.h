#pragma once

namespace Nork::Renderer::Data
{
	struct PointLight
	{
		glm::vec3 position;
		float dummy;
		glm::vec4 color;
		float linear, quadratic;
		float dummy2[2];
	};
	struct PointShadow
	{
		float bias, biasMin, blur, radius, far;
		float dummies[3];
	};
	struct DirLight
	{
		glm::vec3 direction;
		float dummy;
		glm::vec4 color;
	};
	struct DirShadow
	{
		glm::mat4 VP;
		float bias, biasMin, pcfSize, dummy;
	};
}