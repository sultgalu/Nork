#pragma once

namespace Nork::Renderer
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
		void ClearAll()
		{
			pointLights.clear();
			pointShadows.clear();
			dirLights.clear();
			dirShadows.clear();
		}
		std::vector<PointLight> pointLights;
		std::vector<PointShadow> pointShadows;
		std::vector<DirLight> dirLights;
		std::vector<DirShadow> dirShadows;
	};
}