#include "pch.h"
#include "LightManager.h"
#include "../Utils.h"

namespace Nork::Renderer::Pipeline
{
	void LightManager::Update(std::span<Data::DirLight> dLights, std::span<Data::PointLight> pLights)
	{
		LightShaderData newData;

		for (size_t i = 0; i < dLights.size(); i++)
		{
			newData.DL[i] = dLights[i];
		}

		for (size_t i = 0; i < pLights.size(); i++)
		{
			newData.PL[i] = pLights[i];
		}

		newData.dLightCount = dLights.size();
		newData.pLightCount = pLights.size();
		this->data.Update(newData);
	}
}