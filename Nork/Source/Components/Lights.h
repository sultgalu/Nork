#pragma once

#include "Modules/Renderer/Model/Lights.h"

namespace Nork::Components
{
	struct PointLight : public Renderer::PointLight
	{
		PointLight() 
		{
			position = { 0, 0, 0 };
			color = { 1,1,1,1 };
			SetIntensity(7);
		}

		inline void SetColor(glm::vec4& val) { color = glm::vec4(val); }
		inline void SetPosition(glm::vec3& val) { position = glm::vec3(val); }
		void SetIntensity(uint32_t val);
		inline uint32_t GetIntensity() const { return intensity; }
	private:
		uint32_t intensity;
	};

	struct PointShadow : Renderer::PointShadow
	{
		PointShadow()
			: Renderer::PointShadow(Renderer::PointShadow{ .bias = 0.0057, .biasMin = 0.0004 , .blur = 1, .radius = 0.024, .far = 50, .near = 1, .idx = 0 })
		{}
		// inline void SetBias(float val) { bias = val; }
		// inline void SetBiasMin(float val) { biasMin = val; }
		// inline void SetBlur(float val) { blur = val; }
		// inline void SetRadius(float val) { radius = val; }
		// inline void SetFar(float val) { far = val; }
	};

	struct DirLight : Renderer::DirLight
	{
		DirLight()
			: Renderer::DirLight(Renderer::DirLight{ .direction = glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f)), .color = {1,1,1,1} })
		{}

		inline glm::mat4 GetView() const { return glm::lookAt(glm::vec3(0) - direction, glm::vec3(0), glm::vec3(0.0f, 1.0f, 0.0f)); }
		inline void SetColor(glm::vec4&& val) { color = glm::vec4(val); }
		inline void SetDirection(glm::vec3&& val) { direction = glm::vec3(val); }
	};
	struct DirShadow : Renderer::DirShadow
	{
		DirShadow() 
			: Renderer::DirShadow(Renderer::DirShadow {.VP = glm::identity<glm::mat4>(), .bias = 0.01f,
				.biasMin = 0.01f , .pcfSize = 1})
		{}
		inline void SetBias(float val) { bias = val; }
		inline void SetBiasMin(float val) { biasMin = val; }
		inline void SetPcfSize(float val) { pcfSize = val; }
		inline void RecalcVP(const glm::mat4& view)
		{
			VP = glm::ortho(left, right, bottom, top, near, far) * view;
		}
		float left = -30, right = 30, bottom = -30, top = 30, near = -50, far = 50;
	};
}