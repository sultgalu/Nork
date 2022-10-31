#include "LightFramebufferBuilder.h"
#include "../GLManager.h"
#include "../Texture/TextureBuilder.h"

namespace Nork::Renderer {

	std::shared_ptr<MainFramebuffer> LightFramebufferBuilder::Create()
	{
		Validate();
		CreateAttachments();
		FramebufferBuilder::Validate();
		glGenFramebuffers(1, &handle);
		SetAttachments();

		auto fb = std::make_shared<MainFramebuffer>(handle, width, height, attachments);
		GLManager::Get().fbos[fb->GetHandle()] = fb;
		Logger::Info("Created main framebuffer ", handle);
		return fb;
	}
	void LightFramebufferBuilder::CreateAttachments()
	{
		auto createTexture = [&](TextureFormat format)
		{
			return TextureBuilder()
				.Params(TextureParams::FramebufferTex2DParams())
				.Attributes(TextureAttributes{ .width = (uint32_t)width, .height = (uint32_t)height, .format = format })
				.Create2DEmpty();
		};
		attachments = FramebufferAttachments()
			.Color(createTexture(color), 0)
			.Depth(createTexture(depth));
	}
}
