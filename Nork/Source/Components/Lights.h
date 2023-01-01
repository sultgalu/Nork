#pragma once

#include "Modules/Renderer/Model/Lights.h"

namespace Nork::Components
{
	struct PointLight
	{
		void SetIntensity(uint32_t val);
		inline uint32_t GetIntensity() const { return intensity; }
	
		uint32_t intensity;

		const Renderer::PointLight& Data() const { return *rendererLight; }
		Renderer::PointLight::Writer Data() { return rendererLight->Data(); }
		auto operator->()
		{
			return Data();
		}
		std::shared_ptr<Renderer::PointLight> rendererLight;
	};
	struct PointShadowMap
	{
		char dummy;
	};

	//Renderer::PointShadow(Renderer::PointShadow{ .bias = 0.0057, .biasMin = 0.0004 , .blur = 1, .radius = 0.024, .far = 50, .near = 1, .idx = 0 })
	//Renderer::DirLight(Renderer::DirLight{ .direction = glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f)), .color = {1,1,1,1} })
	//Renderer::DirShadow(Renderer::DirShadow {.VP = glm::identity<glm::mat4>(), .bias = 0.01f, .biasMin = 0.01f , .pcfSize = 1})
	
	struct DirLight
	{
		inline glm::mat4 GetView() const {
			return glm::lookAt(glm::vec3(0) - Data()->direction, glm::vec3(0), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		inline void RecalcVP()
		{
			auto right = position.x + rectangle.x / 2;
			auto left = position.x - rectangle.x / 2;
			auto top = position.y + rectangle.y / 2;
			auto bottom = position.y - rectangle.y / 2;
			auto near = position.z;
			auto far = position.z + rectangle.z;
			Data()->VP = glm::ortho(left, right, bottom, top, near, far) * GetView();
		}
		glm::vec3 position = { 100, 0, 0 };
		glm::vec3 rectangle = { 60, 60, 150 };
		// float left = -30, right = 30, bottom = -30, top = 30, near = -50, far = 100;
		bool sun = false;
		const Renderer::DirLight& Data() const { return *rendererLight; }
		Renderer::DirLight::Writer Data() { return rendererLight->Data(); }
		auto operator->()
		{
			return Data();
		}
		std::shared_ptr<Renderer::DirLight> rendererLight;
	};
	struct DirShadowMap // should only exist if the same component has a dirLight as well
	{
		char dummy;
		void FixTextureRatio(const DirLight&, uint32_t pixelCount);
	};
}