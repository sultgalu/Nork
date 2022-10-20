export module Nork.Renderer:FramebufferBuilder;

export import :Framebuffer;

export namespace Nork::Renderer {
	class FramebufferBuilder
	{
	public:
		std::shared_ptr<Framebuffer> Create();
		FramebufferBuilder& Attachments(FramebufferAttachments attachements);
	protected:
		void SetAttachments();
		void UpdateDrawBuffers();
		void AddColorTexture(GLuint texture, int idx)
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + idx, texture, 0);
		}
		void AddDepthTexture(GLuint texture)
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
		}
		void Validate();
	protected:
		GLuint handle;
		int width = 0, height = 0;
		FramebufferAttachments attachments;
	};
}