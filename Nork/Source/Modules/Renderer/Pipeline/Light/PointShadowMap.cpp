import Nork.Renderer;

#include "../../Config.h"

namespace Nork::Renderer {
	PointShadowMap::PointShadowMap(std::shared_ptr<Shader> shader, uint32_t size, TextureFormat depthFormat)
		: shader(shader)
	{
		auto depth = TextureBuilder()
			.Params(TextureParams::ShadowMapParamsCube())
			.Attributes(TextureAttributes{ .width = size, .height = size, .format = depthFormat })
			.CreateCubeEmpty();
		framebuffer = FramebufferBuilder().Attachments(FramebufferAttachments().Depth(depth)).Create();
	}
	void PointShadowMap::Render(const Data::PointLight& light, const Data::PointShadow& shadow, const std::vector<DrawCommandMultiIndirect>& drawCommands)
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
			shader->SetMat4(("VP[" + std::to_string(i) + "]").c_str(), VP[i]);
		}

		shader->SetFloat("far", shadow.far);
		shader->SetVec3("ligthPos", pos);

		Capabilities()
			.Enable().DepthTest().CullFace();

		for (auto& command : drawCommands)
		{
			command.Draw(*shader);
		}
	}
	std::shared_ptr<TextureCube> PointShadowMap::Get()
	{
		return std::static_pointer_cast<TextureCube>(framebuffer->GetAttachments().depth);
	}

}