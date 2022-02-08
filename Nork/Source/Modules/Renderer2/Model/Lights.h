#pragma once

namespace Nork::Renderer2
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
		float bias, biasMin, blur, radius, far, near;
		int idx;
		float dummies[1];
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
		float bias, biasMin, pcfSize;
		int idx;
	};

	struct LightState
	{
		std::vector<PointLight> pointLights;
		std::vector<PointShadow> pointShadows;
		std::vector<DirLight> dirLights;
		std::vector<DirShadow> dirShadows;
	};
}