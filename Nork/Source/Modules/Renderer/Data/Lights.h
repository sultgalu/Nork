#pragma once

namespace Nork::Renderer::Data
{
	struct PointLight
	{
		glm::vec3 position = { 0, 0, 0 };
		float pad;
		glm::vec4 color = { 1, 1, 1, 1 };
		float linear, quadratic;
		float padding[2];
	};
	struct PointShadow
	{
		float bias = 0.006, biasMin = 0.0001,
			near = 0, far = 50;
		uint32_t shadMap;
		uint32_t padding[3];
	};
	struct DirLight
	{
		glm::vec3 direction = glm::vec3(-1);
		float outOfProjValue = 1;
		glm::vec4 color = glm::vec4(1); // ambient
		glm::vec4 color2 = glm::vec4(1);
		glm::mat4 VP;
	};
	struct DirShadow
	{
		float bias = 0.01f, biasMin = 0.01f;
		uint32_t shadMap;
		uint32_t padding;
	};
}