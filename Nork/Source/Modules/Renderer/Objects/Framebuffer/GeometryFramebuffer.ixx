export module Nork.Renderer:GeometryFramebuffer;

export import :Framebuffer;

export namespace Nork::Renderer {
	class GeometryFramebuffer : public Framebuffer
	{
	public:
		GeometryFramebuffer(GLuint handle, int width, int height, FramebufferAttachments attachments)
			: Framebuffer(handle, width, height, attachments) 
		{}
		std::shared_ptr<Texture2D> Position()
		{
			return std::static_pointer_cast<Texture2D>(attachments.colors[0].first);
		}
		std::shared_ptr<Texture2D> Diffuse()
		{
			return std::static_pointer_cast<Texture2D>(attachments.colors[1].first);
		}
		std::shared_ptr<Texture2D> Normal()
		{
			return std::static_pointer_cast<Texture2D>(attachments.colors[2].first);
		}
		std::shared_ptr<Texture2D> Specular()
		{
			return std::static_pointer_cast<Texture2D>(attachments.colors[3].first);
		}
		std::shared_ptr<Texture2D> Depth()
		{
			return std::static_pointer_cast<Texture2D>(attachments.depth);
		}
	};
}