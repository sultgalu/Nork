module Nork.Renderer;

namespace Nork::Renderer {

	std::shared_ptr<LightFramebuffer> LightFramebufferBuilder::Create()
	{
		Validate();
		width = depth->GetWidth();
		height = depth->GetHeight();
		CreateAttachments();
		FramebufferBuilder::Validate();
		glGenFramebuffers(1, &handle);
		SetAttachments();

		auto fb = std::make_shared<LightFramebuffer>(handle, width, height, attachments);
		GLManager::Get().fbos[fb->GetHandle()] = fb;
		Logger::Info("Created geometry framebuffer ", handle);
		return fb;
	}
	void LightFramebufferBuilder::CreateAttachments()
	{
		attachments = FramebufferAttachments()
			.Color(TextureBuilder()
				.Params(TextureParams::FramebufferTex2DParams())
				.Attributes(TextureAttributes{ .width = (uint32_t)width, .height = (uint32_t)height, .format = color })
				.Create2DEmpty(), 0)
			.Depth(depth);
	}
}
