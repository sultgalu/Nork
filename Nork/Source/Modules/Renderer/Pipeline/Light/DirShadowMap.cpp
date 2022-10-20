#include "pch.h"
#include "DirShadowMap.h"
#include "../../State/Capabilities.h"	
#include "../../Config.h"
import Nork.Renderer;

namespace Nork::Renderer {
	DirShadowMap::DirShadowMap(std::shared_ptr<Shader> shader, uint32_t width, uint32_t height, TextureFormat depthFormat)
		: shader(shader)
	{
		auto depth = TextureBuilder()
			.Params(TextureParams::ShadowMapParams2D())
			.Attributes(TextureAttributes{ .width = width, .height = height, .format = depthFormat })
			.Create2DEmpty();
		framebuffer = FramebufferBuilder().Attachments(FramebufferAttachments().Depth(depth)).Create();
	}
	void DirShadowMap::Render(const Data::DirLight& light, const Data::DirShadow& shadow, const std::vector<DrawCommandMultiIndirect>& drawCommands)
	{
		framebuffer->Bind().SetViewport().Clear();
		shader->Use().SetMat4("VP", light.VP);

		Capabilities()
			.Enable().DepthTest().CullFace();

		for (auto& command : drawCommands)
		{
			command.Draw(*shader);
		}
	}
	std::shared_ptr<Texture2D> DirShadowMap::Get()
	{
		return std::static_pointer_cast<Texture2D>(framebuffer->GetAttachments().depth);
	}
}
