#pragma once


namespace Nork::Renderer {
	class GLObject
	{
	public:
		inline bool IsCreated() { return handle != 0; }
		GLuint GetHandle() { 
			return handle; }
	protected:
		GLuint handle = 0;
	};
}