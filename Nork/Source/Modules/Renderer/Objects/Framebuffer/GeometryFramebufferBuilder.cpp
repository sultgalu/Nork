module Nork.Renderer;

namespace Nork::Renderer {

	std::shared_ptr<GeometryFramebuffer> GeometryFramebufferBuilder::Create()
	{
		Validate();
		CreateAttachments();
		FramebufferBuilder::Validate();
		glGenFramebuffers(1, &handle);
		SetAttachments();

		auto fb = std::make_shared<GeometryFramebuffer>(handle, width, height, attachments);
		GLManager::Get().fbos[fb->GetHandle()] = fb;
		Logger::Info("Created geometry framebuffer ", handle);
		return fb;
	}
	void GeometryFramebufferBuilder::CreateAttachments()
	{
		auto createTexture = [&](TextureFormat format)
		{
			return TextureBuilder()
				.Params(TextureParams::FramebufferTex2DParams())
				.Attributes(TextureAttributes{ .width = (uint32_t)width, .height = (uint32_t)height, .format = format })
				.Create2DEmpty();
		};
		attachments = FramebufferAttachments()
			.Color(createTexture(position), 0)
			.Color(createTexture(diffuse), 1)
			.Color(createTexture(normal), 2)
			.Color(createTexture(specular), 3)
			.Depth(createTexture(depth));
	}
}
