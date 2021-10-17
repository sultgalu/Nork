#pragma once

#include "../Data/Ligths.h"
#include "../Data/Mesh.h"
#include "../Data/Shader.h"
#include "SharedData.h"
#include "Framebuffer.h"

namespace Nork::Renderer::Pipeline
{
	class LightManager
	{
	public:
		LightManager() = default;
		void Update(std::span<std::pair<Data::DirLight, Data::DirShadow>> dLigthswithShad, std::span<std::pair<Data::PointLight, Data::PointShadow>> pLigthsWithShad,
			std::span<Data::DirLight> dLigths, std::span<Data::PointLight> pLigths);
		void DrawPointShadowMap(const Data::PointLight& light, const Data::PointShadow& shadow, const std::span<Data::Model> models, ShadowFramebuffer& fb, Data::Shader pShadowMapShader);
		void DrawDirShadowMap(const Data::DirLight& light, const Data::DirShadow& shadow, const std::span<Data::Model> models, ShadowFramebuffer& fb, Data::Shader dShadowMapShader);
	private:
		LightShaderDataStorage data;
	};
}

