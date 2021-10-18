#include "Framebuffer.h"
#include "../Config.h"

namespace Nork::Renderer
{
	static FramebufferBase defaultFb = FramebufferBase{ .fbo = 0, .width = 1920, .height = 1080 };
	static FramebufferBase current = defaultFb;

	FramebufferBase FramebufferBase::GetCurrentInUse()
	{
		return current;
	}

	void FramebufferBase::UseDefault()
	{
		defaultFb.Use();
	}

	void FramebufferBase::Use()
	{
		if constexpr (Config::AssertSyncronization)
		{
			GLint glCurrent = 0;
			glGetIntegerv(GL_FRAMEBUFFER_BINDING, &glCurrent);
			if (current.fbo != glCurrent)
			{
				MetaLogger().Error("Currently bound FBO(", glCurrent, ") is not the one cached(", current.fbo, ").");
				current = FramebufferBase{ .fbo = (GLuint)glCurrent };
			}
		}

		if (current.fbo != fbo)
		{
			if (current.width != width || current.height != height)
				glViewport(0, 0, width, height);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			current = *this;
		}
		else
		{
			Logger::Warning("Trying to make redundant FBO binding. FBO=", fbo, " is already bound.");
		}
	}
	void FramebufferBase::Clear(GLenum clearBits)
	{
		glClear(clearBits);
	}
	void FramebufferBase::ClearAndUse(GLenum clearBits)
	{
		Use();
		Clear(clearBits);
	}
}