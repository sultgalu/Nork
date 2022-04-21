#pragma once

namespace Nork::Renderer::Data
{
	struct PointLight
	{
		glm::vec3 position = { 0, 0, 0 };
		float dummy;
		glm::vec4 color = { 1, 1, 1, 1 };
		float linear, quadratic;
		float padding[2];
	};
	struct PointShadow
	{
		float bias = 0.0057, biasMin = 0.0004,
			near = 1, far = 50;
		uint64_t shadMap;
		uint64_t padding;
	};
	struct DirLight
	{
		glm::vec3 direction;
		float outOfProjValue = 1;
		glm::vec4 color;
		glm::vec4 color2 = glm::vec4(0);
		glm::mat4 VP;
	};
	struct DirShadow
	{
		float bias = 0.01f, biasMin = 0.01f;
		uint64_t shadMap;
	};
}