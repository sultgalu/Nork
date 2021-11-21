#pragma once

#include "Modules/Renderer/Data/Ligths.h"

namespace Nork::Components
{
	template<typename T>
	class Wrapper
	{
	public:
		inline T& GetMutableData() { return data; }
		inline const T& GetData() const { return data; }
	protected:
		Wrapper(T&& t) : data(t) {}
		Wrapper()
		{
			MetaLogger().Warning("Default constructor for type ", typeid(T).name, ".\n\tYou might want to set default values for it.");
		}
		T data;
	};

	struct PointLight : public Wrapper<Renderer::PointLight>
	{
		PointLight() 
			: Wrapper(Renderer::PointLight{ .position = { 0,0,0 }, .color = { 1,1,1,1 }})
		{
			SetIntensity(7);
		}

		inline void SetColor(glm::vec4& val) { data.color = glm::vec4(val); }
		inline void SetPosition(glm::vec3& val) { data.position = glm::vec3(val); }
		// val must be between 0 and PointLight.linears.size() - 1
		inline void SetIntensity(uint32_t val)
		{
			for (size_t i = 0; i < ranges.size(); i++)
			{
				if (val <= ranges[i])
				{
					if (val == ranges[i])
					{
						data.linear = linears[i];
						data.quadratic = quadratics[i];
					}
					else
					{
						float range = ranges[i] - ranges[i - 1];
						float portion = (val - ranges[i - 1]) / range;
						
						float linearRange = linears[i - 1] - linears[i];
						float quadraticRange = quadratics[i - 1] - quadratics[i];

						data.quadratic = quadratics[i - 1] - portion * quadraticRange;
						data.linear = linears[i - 1] - portion * linearRange;
					}
					break;
				}
			}
			intensity = val;
		}
		inline uint32_t GetPower() { return intensity; }

		inline static constexpr auto ranges = std::array<uint32_t, 14>({ 0, 7, 13, 20, 32, 50, 65, 100, 160, 200, 325, 600, 3250, std::numeric_limits<uint32_t>::max() });
		inline static constexpr auto linears = std::array<float, 14>({ 10, 0.7f, 0.35f, 0.22f, 0.14f, 0.09f, 0.07f, 0.045f, 0.027f, 0.022f, 0.014f, 0.007f, 0.0014f, std::numeric_limits<float>::max() / 2 });
		inline static constexpr auto quadratics = std::array<float, 14>({ 20, 1.8f, 0.44f, 0.20f, 0.07f, 0.032f, 0.017f, 0.0075f, 0.0028f, 0.0019f, 0.0007f, 0.0002f, 0.000007f, std::numeric_limits<float>::max() });
		inline static constexpr uint32_t maxPower = ranges.back();
	private:
		uint32_t intensity;
	};

	struct PointShadow : Wrapper<Renderer::PointShadow>
	{
		PointShadow()
			: Wrapper(Renderer::PointShadow{ .bias = 0.0057, .biasMin = 0.0004 , .blur = 1, .radius = 0.024, .far = 50, .near = 1, .idx = 0 })
		{
		}
		inline void SetBias(float val) { data.bias = val; }
		inline void SetBiasMin(float val) { data.biasMin = val; }
		inline void SetBlur(float val) { data.blur = val; }
		inline void SetRadius(float val) { data.radius = val; }
		inline void SetFar(float val) { data.far = val; }
	};

	struct DirLight : Wrapper<Renderer::DirLight>
	{
		DirLight()
			: Wrapper(Renderer::DirLight{ .direction = glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f)), .color = {1,1,1,1} })
		{
		}

		inline glm::mat4 GetView() const { return glm::lookAt(glm::vec3(0) - data.direction, glm::vec3(0), glm::vec3(0.0f, 1.0f, 0.0f)); }
		inline void SetColor(glm::vec4&& val) { data.color = glm::vec4(val); }
		inline void SetDirection(glm::vec3&& val) { data.direction = glm::vec3(val); }
	};
	struct DirShadow : Wrapper<Renderer::DirShadow>
	{
		DirShadow() 
			: Wrapper(Renderer::DirShadow {.VP = glm::identity<glm::mat4>(), .bias = 0.01f,
				.biasMin = 0.01f , .pcfSize = 1})
		{
		}
		inline void SetBias(float val) { data.bias = val; }
		inline void SetBiasMin(float val) { data.biasMin = val; }
		inline void SetPcfSize(float val) { data.pcfSize = val; }
		inline void RecalcVP(const glm::mat4& view)
		{
			data.VP = glm::ortho(left, right, bottom, top, near, far) * view;
		}
		float left = -30, right = 30, bottom = -30, top = 30, near = -50, far = 50;
	};
}