#pragma once

#include "Modules/Renderer/World.h"
#include "Modules/Renderer/Model/ShadowMap.h"

namespace Nork::Components
{
	struct PointLight
	{
		Renderer::PointLightRef light;
		void SetIntensity(uint32_t val);
		inline uint32_t GetIntensity() const { return intensity; }
	private:
		uint32_t intensity;
	};
	struct PointShadowMap
	{
		Renderer::PointShadowMap map;
	};

	//Renderer::PointShadow(Renderer::PointShadow{ .bias = 0.0057, .biasMin = 0.0004 , .blur = 1, .radius = 0.024, .far = 50, .near = 1, .idx = 0 })
	//Renderer::DirLight(Renderer::DirLight{ .direction = glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f)), .color = {1,1,1,1} })
	//Renderer::DirShadow(Renderer::DirShadow {.VP = glm::identity<glm::mat4>(), .bias = 0.01f, .biasMin = 0.01f , .pcfSize = 1})
	
	struct DirLight
	{
		Renderer::DirLightRef light;
		inline glm::mat4 GetView() const { return glm::lookAt(glm::vec3(0) - light->direction, glm::vec3(0), glm::vec3(0.0f, 1.0f, 0.0f)); }
		inline void RecalcVP()
		{
			auto right = position.x + rectangle.x / 2;
			auto left = position.x - rectangle.x / 2;
			auto top = position.y + rectangle.y / 2;
			auto bottom = position.y - rectangle.y / 2;
			auto near = position.z;
			auto far = position.z + rectangle.z;
			light->VP = glm::ortho(left, right, bottom, top, near, far) * GetView();
		}
		glm::vec3 position = { 100, 0, 0 };
		glm::vec3 rectangle = { 60, 60, 150 };
		// float left = -30, right = 30, bottom = -30, top = 30, near = -50, far = 100;
		bool sun = false;
	};
	struct DirShadowMap // should only exist if the same component has a dirLight as well
	{
		Renderer::DirShadowMap map;
		void FixTextureRatio(const DirLight&, uint32_t pixelCount);
	};
}