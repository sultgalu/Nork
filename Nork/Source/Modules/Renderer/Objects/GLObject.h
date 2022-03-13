#pragma once

namespace Nork::Renderer {
	class GLObject
	{
	public:
		GLObject(GLuint handle) : handle(handle) {}
		inline bool IsCreated() { return handle != 0; }
		GLuint GetHandle() { return handle; }
		void SetDebugLabel(std::string label)
		{
			glObjectLabel(GetIdentifier(), handle, -1, label.c_str());
		}

		GLObject(const GLObject&) = delete;
		GLObject(GLObject&) = delete;
		GLObject& operator=(const GLObject&) = delete;
		GLObject& operator=(GLObject&) = delete;
	protected:
		GLuint handle = 0;
	protected:
		virtual GLenum GetIdentifier() = 0;
	};
}