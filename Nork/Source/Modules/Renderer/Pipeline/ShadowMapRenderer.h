#pragma once

#include "../Config.h"
#include "../Model/Lights.h"
#include "../Model/Model.h"
#include "../Objects/Shader/Shader.h"
#include "../Objects/Framebuffer/Framebuffer.h"
#include "../State/Capabilities.h"

namespace Nork::Renderer {
	class ShadowMapRenderer
	{
	public:
		static void RenderPointLightShadowMap(const PointLight& light, const PointShadow& shadow, ModelIterator iterator, Framebuffer& fb, Shader& shader)
		{
			fb.Bind().SetViewport().Clear();
			shader.Use();

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
				shader.SetMat4("VP[" + std::to_string(i) + "]", VP[i]);
			}

			shader.SetFloat("far", shadow.far);
			shader.SetVec3("ligthPos", pos);
			
			Capabilities()
				.Enable().DepthTest().CullFace();

			iterator([&](Model& model)
				{
					model.DrawTextureless(shader);
				});
		}
		static void RenderDirLightShadowMap(const DirLight& light, const DirShadow& shadow, ModelIterator iterator, Framebuffer& fb, Shader& shader)
		{
			fb.Bind().SetViewport().Clear();
			shader.Use().SetMat4("VP", shadow.VP);

			Capabilities()
				.Enable().DepthTest().CullFace();

			iterator([&](Model& model)
				{
					model.DrawTextureless(shader);
				});
		}
		static void BindDirShadowMap(const DirShadow& shadow, Framebuffer& fb)
		{
			fb.GetAttachments().depth->Bind2D(shadow.idx + Config::LightData::dirShadowBaseIndex);
		}
		static void BindPointShadowMap(const PointShadow& shadow, Framebuffer& fb)
		{
			fb.GetAttachments().depth->BindCube(shadow.idx + Config::LightData::pointShadowBaseIndex);
		}
	};
}