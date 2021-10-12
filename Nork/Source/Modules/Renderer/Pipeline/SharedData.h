#pragma once

#include "Modules/Renderer/Utils.h"
#include "Modules/Renderer/Data/Ligths.h"
#include "Modules/Renderer/Config.h"

namespace Nork::Renderer
{
	template<typename T, int idx>
	class ShaderDataStorage
	{
	public:
		ShaderDataStorage()
		{
			ubo = Utils::Other::CreateUBO(idx, sizeof(T), GL_DYNAMIC_DRAW);
		}
		void Update(T& data)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, ubo);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(T), &data);
		}
		template<size_t from, size_t size>
		void UpdateAt(std::any& data)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, ubo);
			glBufferSubData(GL_UNIFORM_BUFFER, from, size, &data);
		}
		template<size_t from, typename M>
		void UpdateAt(M& data)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, ubo);
			glBufferSubData(GL_UNIFORM_BUFFER, from, sizeof(M), &data);
		}
		/*template<typename M>
		void UpdateAt(M& data, T::* memPtr)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, ubo);
			glBufferSubData(GL_UNIFORM_BUFFER, memPtr, sizeof(M), &data);
		}*/
	private:
		GLuint ubo;
	};

	struct CommonShaderData
	{
		glm::mat4 VP, view, projection;
	};

	struct LightShaderData
	{
		using c = Config::LightData;

		static constexpr size_t DSLimit = c::dirShadowsLimit;
		static constexpr size_t PSLimit = c::pointShadowsLimit;
		static constexpr size_t DLLimit = c::dirLightsLimit;
		static constexpr size_t PLLimit = c::pointLightsLimit;

		float dLightCount, dShadowCount; // lightCount means lights without shadows
		float pLightCount, pShadowCount;

		Data::DirShadow DS[DSLimit];
		Data::PointShadow PS[PSLimit];

		Data::DirLight DL[DLLimit];
		Data::PointLight PL[PLLimit];

	};

	using conf = Config::UBOIdx;
	using CommonShaderDataStorage = ShaderDataStorage<CommonShaderData, conf::common>;
	using LightShaderDataStorage = ShaderDataStorage<LightShaderData, conf::lights>;
}