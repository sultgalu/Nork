#include "pch.h"
#include "LightManager.h"
#include "../Utils.h"

namespace Nork::Renderer::Pipeline
{
	using namespace Data;

	void LightManager::Update(std::span<std::pair<DirLight, DirShadow>> dLigthswithShad, std::span<std::pair<PointLight, PointShadow>> pLigthsWithShad,
		std::span<DirLight> dLigths, std::span<PointLight> pLigths)
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

	void LightManager::DrawPointShadowMap(const Data::PointLight& light, const Data::PointShadow& shadow, const std::span<Data::Model> models, PointShadowFramebuffer& fb, Data::Shader& pShadowMapShader)
	{
		fb.ClearAndUse();
		pShadowMapShader.Use();

		auto& pos = light.position;

		glm::mat4 projection = glm::perspective(glm::radians(90.f), 1.0f, shadow.near, shadow.far);

		std::vector<glm::mat4> VP;
		// Adding view
		VP.push_back(glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		VP.push_back(glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		VP.push_back(glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		VP.push_back(glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		VP.push_back(glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		VP.push_back(glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		for (int i = 0; i < VP.size(); i++)
		{
			VP[i] = projection * VP[i];
			pShadowMapShader.SetMat4("VP[" + std::to_string(i) + "]", VP[i]);
		}

		pShadowMapShader.SetFloat("far", shadow.far);
		pShadowMapShader.SetVec3("ligthPos", pos);

		for (size_t i = 0; i < models.size(); i++)
		{
			auto& meshes = models[i].first;
			auto& mat = models[i].second;

			pShadowMapShader.SetMat4("model", mat);
			for (int i = 0; i < meshes.size(); i++)
			{
				meshes[i].Draw();
			};
		}
	}
	void LightManager::DrawDirShadowMap(const Data::DirLight& light, const Data::DirShadow& shadow, const std::span<Data::Model> models, DirShadowFramebuffer& fb, Data::Shader& dShadowMapShader)
	{
		fb.ClearAndUse();
		dShadowMapShader.Use();
		dShadowMapShader.SetMat4("VP", shadow.VP);

		for (size_t i = 0; i < models.size(); i++)
		{
			auto& meshes = models[i].first;
			auto& mat = models[i].second;

			dShadowMapShader.SetMat4("model", mat);
			for (int i = 0; i < meshes.size(); i++)
			{
				meshes[i].Draw();
			};
		}
	}
}