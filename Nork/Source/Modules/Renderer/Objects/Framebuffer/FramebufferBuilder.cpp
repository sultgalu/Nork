module Nork.Renderer;

namespace Nork::Renderer {
	std::shared_ptr<Framebuffer> FramebufferBuilder::Create()
	{
		Validate();
		glGenFramebuffers(1, &handle);
		SetAttachments();
		auto fb = std::make_shared<Framebuffer>(handle, width, height, attachments);
		GLManager::Get().fbos[fb->GetHandle()] = fb;
		Logger::Info("Created framebuffer ", handle);
		return fb;
	}
	FramebufferBuilder& FramebufferBuilder::Attachments(FramebufferAttachments attachements)
	{
		this->attachments = attachements;
		if (attachments.depth != nullptr)
		{
			this->width = attachments.depth->GetWidth();
			this->height = attachments.depth->GetHeight();
		}
		else
		{
			this->width = attachments.colors[0].first->GetWidth();
			this->height = attachments.colors[0].first->GetHeight();
		}
		return *this;
	}
	void FramebufferBuilder::SetAttachments()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, handle);
		if (attachments.depth != nullptr)
		{
			AddDepthTexture(attachments.depth->GetHandle());
		}
		for (auto att : attachments.colors)
		{
			if (att.first->GetWidth() != width
				|| att.first->GetHeight() != height)
			{
				Logger::Error("A framebuffer's attachments should be of the same resolution");
			}
			AddColorTexture(att.first->GetHandle(), att.second);
		}
		UpdateDrawBuffers();
	}
	void FramebufferBuilder::UpdateDrawBuffers()
	{
		if (attachments.colors.size() > 0)
		{
			auto buf = std::vector<GLenum>(attachments.colors.size());
			for (size_t i = 0; i < buf.size(); i++)
				buf[i] = GL_COLOR_ATTACHMENT0 + attachments.colors[i].second;
			glDrawBuffers(buf.size(), buf.data());
		}
		else
		{
			glDrawBuffer(GL_NONE);
		}
	}
	void FramebufferBuilder::Validate()
	{
		if (width <= 0 || height <= 0
			|| (attachments.colors.size() == 0 && attachments.depth == nullptr))
		{
			std::abort();
		}

		std::unordered_set<int> colorIdxs;
		for (auto& color : attachments.colors)
		{
			if (colorIdxs.contains(color.second))
			{
				MetaLogger().Error(color.second, " color idx is already set. This is probably not what you want.");
			}
			else
			{
				colorIdxs.insert(color.second);
			}
		}
	}
}
