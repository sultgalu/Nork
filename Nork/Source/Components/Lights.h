#pragma once

#include "Modules/Renderer/Data/Ligths.h"

namespace Nork::Components
{
	using namespace Renderer;

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

	struct PointLight : public Wrapper<Data::PointLight>
	{
		PointLight() 
			: Wrapper(Data::PointLight{ .position = { 0,0,0 }, .color = { 1,1,1,1 }, .linear = linears[1], .quadratic = quadratics[maxPower]})
		{
			power = maxPower;
		}

		inline void SetColor(glm::vec4& val) { data.color = glm::vec4(val); }
		inline void SetPosition(glm::vec3& val) { data.position = glm::vec3(val); }
		// val must be between 0 and PointLight.linears.size() - 1
		inline void SetPower(int val)
		{
			if (val <= maxPower)
			{
				power = val;
				data.linear = linears[power];
				data.quadratic = quadratics[power];
			}
			else MetaLogger().Error("Trying to set light power (to ", val, ") higher than the maximum value (", maxPower ,").");
		}
		inline size_t GetPower() { return power; }

		inline static constexpr auto linears = std::array<float, 10>({ 0.22f, 0.14f, 0.09f, 0.07f, 0.045f, 0.027f, 0.022f, 0.014f, 0.007f, 0.0014f });
		inline static constexpr auto quadratics = std::array<float, 10>({ 0.20f, 0.07f, 0.032f, 0.017f, 0.0075f, 0.0028f, 0.0019f, 0.0007f, 0.0002f, 0.000007f });
		inline static constexpr size_t maxPower = linears.size() - 1;
	private:
		size_t power;
	};

	struct PointShadow : Wrapper<Data::PointShadow>
	{
		PointShadow()
			: Wrapper(Data::PointShadow{ .bias = 0.0057, .biasMin = 0.0004 , .blur = 1, .radius = 0.024, .far = 50, .near = 1, .idx = 0 })
		{
		}
		inline void SetBias(float val) { data.bias = val; }
		inline void SetBiasMin(float val) { data.biasMin = val; }
		inline void SetBlur(float val) { data.blur = val; }
		inline void SetRadius(float val) { data.radius = val; }
		inline void SetFar(float val) { data.far = val; }
	};

	struct DirLight : Wrapper<Data::DirLight>
	{
		DirLight()
			: Wrapper(Data::DirLight{ .direction = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)), .color = {1,1,1,1} })
		{
		}

		inline glm::mat4 GetView() const { return glm::lookAt(glm::vec3(0) - data.direction, glm::vec3(0), glm::vec3(0.0f, 1.0f, 0.0f)); }
		inline void SetColor(glm::vec4& val) { data.color = glm::vec4(val); }
		inline void SetDirection(glm::vec3& val) { data.direction = glm::vec3(val); }
	};
	struct DirShadow : Wrapper<Data::DirShadow>
	{
		DirShadow() 
			: Wrapper(Data::DirShadow {.VP = glm::identity<glm::mat4>(), .bias = 0.01f,
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