#include "ShadowMap.h"
#include "../Objects/Texture/TextureBuilder.h"
#include "../Objects/Framebuffer/FramebufferBuilder.h"

namespace Nork::Renderer {
	void DirShadowMap::SetFramebuffer(uint32_t w, uint32_t h, TextureFormat format)
	{
		auto depth = TextureBuilder()
			.Params(TextureParams::ShadowMapParams2D())
			.Attributes(TextureAttributes{ .width = w, .height = h, .format = format })
			.Create2DEmpty();
		fb = FramebufferBuilder().Attachments(FramebufferAttachments().Depth(depth)).Create();
		shadow->shadMap = fb->Depth()->GetBindlessHandle();
	}
	void PointShadowMap::SetFramebuffer(uint32_t res, TextureFormat format)
	{
		auto depth = TextureBuilder()
			.Params(TextureParams::ShadowMapParamsCube())
			.Attributes(TextureAttributes{ .width = res, .height = res, .format = format })
			.CreateCubeEmpty();
		fb = FramebufferBuilder().Attachments(FramebufferAttachments().Depth(depth)).Create();
		shadow->shadMap = fb->Depth()->GetBindlessHandle();
	}
	std::shared_ptr<Texture2D> DirShadowMap::Texture()
	{
		return std::static_pointer_cast<Texture2D>(fb->GetAttachments().depth);
	}
	std::shared_ptr<TextureCube> PointShadowMap::Texture()
	{
		return std::static_pointer_cast<TextureCube>(fb->GetAttachments().depth);
	}
}