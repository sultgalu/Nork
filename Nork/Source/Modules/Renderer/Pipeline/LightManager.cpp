#include "pch.h"
#include "LightManager.h"
#include "../Utils.h"

namespace Nork::Renderer::Pipeline
{
	void LightManager::Update(std::span<std::pair<Data::DirLight, Data::DirShadow>> dLigthswithShad, std::span<std::pair<Data::PointLight, Data::PointShadow>> pLigthsWithShad,
		std::span<Data::DirLight> dLigths, std::span<Data::PointLight> pLigths)
	{
		LightShaderData newData;

		for (size_t i = 0; i < dLigthswithShad.size(); i++)
		{
			newData.DL[i] = dLigthswithShad[i].first;
			newData.DS[i] = dLigthswithShad[i].second;
		}

		for (size_t i = 0; i < pLigthsWithShad.size(); i++)
		{
			newData.PL[i] = pLigthsWithShad[i].first;
			newData.PS[i] = pLigthsWithShad[i].second;
		}

		for (size_t i = dLigthswithShad.size(); i < dLigths.size(); i++)
		{
			newData.DL[i] = dLigths[i];
		}

		for (size_t i = pLigthsWithShad.size(); i < pLigths.size(); i++)
		{
			newData.PL[i] = pLigths[i];
		}

		newData.dLightCount = dLigths.size();
		newData.pLightCount = pLigths.size();
		newData.dShadowCount = dLigthswithShad.size();
		newData.pShadowCount = pLigthsWithShad.size();
		this->data.Update(newData);
	}
}