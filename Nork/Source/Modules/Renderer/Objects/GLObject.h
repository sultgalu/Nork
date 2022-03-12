#pragma once

namespace Nork::Renderer {
	class GLObject
	{
	public:
		GLObject(GLuint handle) : handle(handle) {}
		inline bool IsCreated() { return handle != 0; }
		GLuint GetHandle() { return handle; }
		GLObject(const GLObject&) = delete;
		GLObject(GLObject&) = delete;
		GLObject& operator=(const GLObject&) = delete;
		GLObject& operator=(GLObject&) = delete;
	protected:
		GLuint handle = 0;
	};

	class GLObjectOld
	{
	public:
		inline bool IsCreated() { return handle != 0; }
		GLuint GetHandle() { return handle; }
	protected:
		GLuint handle = 0;
	};
}