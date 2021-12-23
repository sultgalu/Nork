#include "pch.h"
#include "LightManager.h"
#include "../Utils.h"

namespace Nork::Renderer
{
	LightManager::LightManager(Shader shader)
		: commonUBO(0), dirLightUBO(1), dirShadowUBO(2), 
		pointLightUBO(3), pointShadowUBO(4), pLightIndicesSSBO(10), 
		pLightRangesSSBO(11), configSSBO(12), lightCullShader(shader)
	{
	}

	void LightManager::Update()
	{
		commonUBO.Bind();
		commonUBO.SetData(&commonData, 1, 0);
	}

	void LightManager::DrawPointShadowMap(const PointLight& light, const PointShadow& shadow, const std::span<Model> models, PointShadowFramebuffer& fb, Shader& pShadowMapShader)
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
	void LightManager::DrawDirShadowMap(const DirLight& light, const DirShadow& shadow, const std::span<Model> models, DirShadowFramebuffer& fb, Shader& dShadowMapShader)
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
	void LightManager::SetPointLightData(std::span<PointLight> pls, std::span<PointShadow> pss)
	{
		pointLightUBO.Bind();
		pointLightUBO.SetData(pls);
		pointShadowUBO.Bind();
		pointShadowUBO.SetData(pss);

		commonData.pLightCount = pls.size();
		commonData.pShadowCount = pss.size();
	}
	void LightManager::SetPointLightData(std::span<PointLight> pls, std::span<PointShadow> pss, glm::mat4 view, glm::mat4 proj)
	{
		SetPointLightData(pls, pss);
		PointLightCulling(pls, view, proj);
	}
	void LightManager::SetDirLightData(std::span<DirLight> dls, std::span<DirShadow> dss)
	{
		dirLightUBO.Bind();
		dirLightUBO.SetData(dls);
		dirShadowUBO.Bind();
		dirShadowUBO.SetData(dss);

		commonData.dLightCount = dls.size();
		commonData.dShadowCount = dss.size();
	}

	static GLuint tex;

	GLuint LightManager::GetDebug()
	{
		/*TextureData data;
		data.channels = 3;
		data.format = TextureFormat::RGB16F;
		data.width = 1920;
		data.height = 1080;
		GLuint tex = Resource::CreateTexture(data).id;*/

		return tex;
	}

	static Shader cullDisplayShader;

	void LightManager::SetDebug(Shader shader)
	{
		cullDisplayShader = shader;
	}

	static std::vector<uint32_t> lightIndicesCPU;
	static std::vector<glm::uvec2> rangesCPU;
	static std::vector<glm::uvec2> affectedParts;
	static uint32_t counter;

	void LightCullCpu(std::span<PointLight> lights, glm::mat4 view, glm::mat4 iP, glm::mat4 proj)
	{
		counter = 0;
		affectedParts.clear();
		glm::uvec2 size(80, 45);
		for (size_t i = 0; i < size.x; i++)
		{
			for (size_t j = 0; j < size.y; j++)
			{
				glm::vec2 rectSize = glm::vec2(1.0f / (float)i, 1.0f / (float)i);

				auto frag = glm::vec2((float)i / (float)size.x, (float)j / (float)size.y);
				frag.x = frag.x * 2 - 1.0f;
				frag.y = frag.y * 2 - 1.0f;
				auto centerNear1 = (iP * glm::vec4(frag, 0, 1));
				auto centerNear = glm::vec3(centerNear1) / centerNear1.w;

				auto centerFar1 = (iP * glm::vec4(frag, 1, 1));
				auto centerFar = glm::vec3(centerFar1) / centerFar1.w;

				auto partVec = centerFar - centerNear;

				std::vector<uint32_t> sharedIndices;

				for (size_t k = 0; k < lights.size(); k++)
				{
					PointLight light = lights[k];
					glm::vec3 lightViewPos = glm::vec3((view * glm::vec4(light.position, 1)));
					// linear * distance + quadratic * distance * distance - 1000 = 0
					// quadratic * distance * distance + linear * distance - 1000 = 0
					// a * x * x + b * x + c = 0 -> a = quadratic, b = linear, c = -accuracy
					// x = -b + sqrt(b*b - 4*a*c) / (2*a)
					float linear = light.linear;
					float quadratic = light.quadratic;
					float accuracy = 1000.0f;
					float distance = -linear + glm::sqrt(linear * linear + 4 * quadratic * accuracy) / (2 * quadratic);

					auto plane = glm::cross(partVec, lightViewPos - centerNear);
					auto partLineToLightDir = glm::cross(plane, partVec);
					auto actualDistance = glm::dot(glm::normalize(partLineToLightDir), (lightViewPos - centerNear));

					if (distance > actualDistance)
					{
						sharedIndices.push_back(k);
					}
				}
				uint32_t offset = counter;
				counter += sharedIndices.size();
				if (counter > 0)
					affectedParts.push_back(glm::uvec2(i, j));
				for (size_t f = 0; f < sharedIndices.size(); f++)
				{
					lightIndicesCPU[offset + f] = sharedIndices[f];
				}
				rangesCPU[j * size.x + i] = glm::uvec2(offset, sharedIndices.size());
			}
		}
	}

	void LightManager::PointLightCulling(std::span<PointLight> lights, glm::mat4 view, glm::mat4 proj)
	{
		auto profiler = Profiler::ProfilerBuilder()
			.AppendScope("Renderer").AppendScope("Light").AppendScope("Cull")
			.ExpectMillis(1)
			.GetScopedProfiler();

		auto p = Profiler::GetProfiler(0.01f, "Renderer", "Light", "Cull", "glFinishInitial").Start();
		//glFinish();
		p.Stop();

		static constexpr auto baseRes = glm::uvec2(16, 9);

		auto res = baseRes * glm::uvec2(cullQ);
		uint32_t partCount = res.x * res.y;
		uint32_t maxLightCount = lights.size() * partCount;

		if constexpr (true)
		{
			p = Profiler::GetProfiler(0.01f, "Renderer", "Light", "Cull", "Setup").Start();
			pLightIndicesSSBO.Bind();
			pLightIndicesSSBO.Resize(maxLightCount * sizeof(uint32_t));

			pLightRangesSSBO.Bind();
			pLightRangesSSBO.Resize(partCount * sizeof(glm::uvec2));

			LightCullConfig config{
				.V = view,
				.iP = glm::inverse(proj),
				.lightCount = (uint32_t)lights.size(),
				.atomicCounter = 0,
				.cullRes = res,
			};
			configSSBO.Bind();
			configSSBO.SetData(&config, 1);
			//lightsSSBO.Data(lights);
			//glFinish();
			p.Stop();
			

			p = Profiler::GetProfiler(0.01f, "Renderer", "Light", "Cull", "DispatchCompute").Start();
			lightCullShader.Use();
			glDispatchCompute(res.x, res.y, 1);
			//glFinish();
			p.Stop();

			/*static std::vector<uint32_t> inds;
			inds.resize(maxLightCount);
			pLightIndicesSSBO.Bind();
			pLightIndicesSSBO.GetData(inds);

			static std::vector<glm::uvec2> rgs;
			rgs.resize(partCount);
			pLightRangesSSBO.Bind();
			pLightRangesSSBO.GetData(rgs);*/
		}
		else
		{
			lightIndicesCPU.resize(maxLightCount);
			rangesCPU.resize(partCount);
			LightCullCpu(lights, view, glm::inverse(proj), proj);
		}

		p = Profiler::GetProfiler(0.01f, "Renderer", "Light", "Cull", "drawCullDebug").Start();
		static Framebuffer<TextureFormat::RGBA16F> fb(1920, 1080);
		fb.ClearAndUse();
		cullDisplayShader.Use();

		glDisable(GL_DEPTH_TEST);
		Utils::Draw::Quad();
		glEnable(GL_DEPTH_TEST);
		//glFinish();
		p.Stop();

		tex = fb.GetColorAttachment(0);
	}
}