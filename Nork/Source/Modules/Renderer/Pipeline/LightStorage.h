#pragma once

#include "../Data/Ligths.h"
#include "../Data/Mesh.h"

namespace Nork::Renderer::Pipeline
{
	class LightStorage
	{
	public:
		LightStorage();
		void OverwriteAll(std::span<Data::DirLight> dLigths, std::span<Data::PointLight> pLigths);
	private:
		GLuint ubo;
	};
}

