#include "pch.h"
#include "DirShadowMap.h"
#include "../../State/Capabilities.h"
#include "../../Config.h"
#include "../../Objects/Texture/TextureBuilder.h"
#include "../../Objects/Framebuffer/FramebufferBuilder.h"

namespace Nork::Renderer {
	DirShadowMap::DirShadowMap(std::shared_ptr<Shader> shader, uint32_t width, uint32_t height, TextureFormat depthFormat)
		: shader(shader)
	{
		auto depth = TextureBuilder()
			.Params(TextureParams::ShadowMapParams())
			.Attributes(TextureAttributes{ .width = width, .height = height, .format = depthFormat })
			.Create2DEmpty();
		framebuffer = FramebufferBuilder().Attachments(FramebufferAttachments().Depth(depth)).Create();
	}
	void DirShadowMap::Render(const DirLight& light, const DirShadow& shadow, ModelIterator iterator)
	{
		framebuffer->Bind().SetViewport().Clear();
		shader->Use().SetMat4("VP", shadow.VP);

		Capabilities()
			.Enable().DepthTest().CullFace();

		iterator([&](Model& model)
			{
				model.DrawTextureless(*shader);
			});
	}

	void DirShadowMap::Bind(const DirShadow& shadow)
	{
		framebuffer->GetAttachments().depth->Bind2D(shadow.idx + Config::LightData::dirShadowBaseIndex);
	}
	std::shared_ptr<Texture2D> DirShadowMap::Get()
	{
		return std::static_pointer_cast<Texture2D>(framebuffer->GetAttachments().depth);
	}
}
