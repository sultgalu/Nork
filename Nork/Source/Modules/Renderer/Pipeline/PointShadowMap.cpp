#include "pch.h"
#include "PointShadowMap.h"
#include "../State/Capabilities.h"
#include "../Config.h"

namespace Nork::Renderer {
	void PointShadowMap::Render(const PointLight& light, const PointShadow& shadow, ModelIterator iterator)
	{
		framebuffer->Bind().SetViewport().Clear();
		shader->Use();

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
			shader->SetMat4("VP[" + std::to_string(i) + "]", VP[i]);
		}

		shader->SetFloat("far", shadow.far);
		shader->SetVec3("ligthPos", pos);

		Capabilities()
			.Enable().DepthTest().CullFace();

		iterator([&](Model& model)
			{
				model.DrawTextureless(*shader);
			});
	}

	void PointShadowMap::Bind(const PointShadow& shadow)
	{
		framebuffer->GetAttachments().depth->BindCube(shadow.idx + Config::LightData::pointShadowBaseIndex);
	}
	std::shared_ptr<TextureCube> PointShadowMap::Get()
	{
		return std::static_pointer_cast<TextureCube>(framebuffer->GetAttachments().depth);
	}

}