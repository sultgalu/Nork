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
		float bias, biasMin;
		int blur;
		float radius, far, near;
		uint64_t shadMap;
	};
	struct DirLight
	{
		glm::vec3 direction;
		float dummy;
		glm::vec4 color;
		glm::mat4 VP;
	};
	struct DirShadow
	{
		float bias, biasMin;
		uint64_t shadMap;
	};
}