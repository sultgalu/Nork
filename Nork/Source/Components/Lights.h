#pragma once

import Nork.Renderer;

namespace Nork::Components
{
	struct PointLight
	{
		std::shared_ptr<Renderer::PointLight> light;
		std::shared_ptr<Renderer::PointShadow> shadow = nullptr;
		void SetIntensity(uint32_t val);
		inline uint32_t GetIntensity() const { return intensity; }
	private:
		uint32_t intensity;
	};
	struct PointShadowRequest
	{
		int pad;
	};

	//Renderer::PointShadow(Renderer::PointShadow{ .bias = 0.0057, .biasMin = 0.0004 , .blur = 1, .radius = 0.024, .far = 50, .near = 1, .idx = 0 })
	//Renderer::DirLight(Renderer::DirLight{ .direction = glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f)), .color = {1,1,1,1} })
	//Renderer::DirShadow(Renderer::DirShadow {.VP = glm::identity<glm::mat4>(), .bias = 0.01f, .biasMin = 0.01f , .pcfSize = 1})
	
	struct DirLight
	{
		std::shared_ptr<Renderer::DirLight> light;
		std::shared_ptr<Renderer::DirShadow> shadow = nullptr;
		inline glm::mat4 GetView() const { return glm::lookAt(glm::vec3(0) - light->direction, glm::vec3(0), glm::vec3(0.0f, 1.0f, 0.0f)); }
		inline void RecalcVP()
		{
			light->VP = glm::ortho(left, right, bottom, top, near, far) * GetView();
		}
		float left = -30, right = 30, bottom = -30, top = 30, near = -50, far = 100;
		bool sun = false;
	};
	struct DirShadowRequest
	{
		int pad;
	};
}