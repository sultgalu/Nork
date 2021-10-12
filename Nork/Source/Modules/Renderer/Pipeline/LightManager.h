#pragma once

#include "../Data/Ligths.h"
#include "../Data/Mesh.h"
#include "SharedData.h"

namespace Nork::Renderer::Pipeline
{
	class LightManager
	{
	public:
		LightManager() = default;
		//void Update(std::span<Data::DirLight> dLigths, std::span<Data::PointLight> pLigths);
		void Update(std::span<std::pair<Data::DirLight, Data::DirShadow>> dLigthswithShad, std::span<std::pair<Data::PointLight, Data::PointShadow>> pLigthsWithShad,
			std::span<Data::DirLight> dLigths, std::span<Data::PointLight> pLigths);
	private:
		LightShaderDataStorage data;
	};
}

