#pragma once

#include "Modules/Renderer/Data/Ligths.h"

namespace Nork::Components
{
	using namespace Renderer;

	template<typename T>
	class Wrapper
	{
	public:
		inline const T& GetData() { return data; }
	protected:
		T data;
	};

	struct PointLight : public Wrapper<Data::PointLight>
	{
		inline void SetColor(glm::vec4& val) { data.color = glm::vec4(val); }
		inline void SetPosition(glm::vec3& val) { data.position = glm::vec3(val); }
		// val must be between 0 and PointLight.linears.size() - 1
		inline void SetPower(int val)
		{
			if (val < linears.size())
			{
				data.linear = linears[val];
				data.quadratic = quadratics[val];
			}
			else MetaLogger().Error("Light power out of bounds.");
		}

		inline static const std::vector<float> linears = std::vector<float>({ 0.22f, 0.14f, 0.09f, 0.07f, 0.045f, 0.027f, 0.022f, 0.014f, 0.007f, 0.0014f });
		inline static const std::vector<float> quadratics = std::vector<float>({ 0.20f, 0.07f, 0.032f, 0.017f, 0.0075f, 0.0028f, 0.0019f, 0.0007f, 0.0002f, 0.000007f });
	};

	struct PointShadow : Wrapper<Data::PointShadow>
	{
		inline void SetBias(float val) { data.bias = val; }
		inline void SetBiasMin(float val) { data.biasMin = val; }
		inline void SetBlur(float val) { data.blur = val; }
		inline void SetRadius(float val) { data.radius = val; }
		inline void SetFar(float val) { data.far = val; }
	};

	struct DirLight : Wrapper<Data::DirLight>
	{
		inline void SetColor(glm::vec4& val) { data.color = glm::vec4(val); }
		inline void SetDirection(glm::vec3& val) { data.direction = glm::vec3(val); }
	};
	struct DirShadow : Wrapper<Data::DirShadow>
	{
		inline void SetBias(float val) { data.bias = val; }
		inline void SetBiasMin(float val) { data.biasMin = val; }
		inline void SetPcfSize(float val) { data.pcfSize = val; }
		inline void RecalcVP(float left, float right, float bottom, float top, float near, float far)
		{
			data.VP = glm::ortho(left, right, bottom, top, near, far);
		}
	};
}