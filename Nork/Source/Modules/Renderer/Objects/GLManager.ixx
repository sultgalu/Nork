#pragma once

export module Nork.Renderer:GLManager;

export import :Buffer;
export import :Texture;
export import :VertexArray;
export import :Shader;
export import :Framebuffer;

export namespace Nork::Renderer {

	class GLManager
	{
	public:
		std::unordered_map<GLuint, std::weak_ptr<Texture2D>> texture2Ds;
		std::unordered_map<GLuint, std::weak_ptr<TextureCube>> textureCubes;
		std::unordered_map<GLuint, std::weak_ptr<Buffer>> buffers;
		std::unordered_map<GLuint, std::weak_ptr<VertexArray>> vaos;
		std::unordered_map<GLuint, std::weak_ptr<Framebuffer>> fbos;
		std::unordered_map<GLuint, std::weak_ptr<Shader>> shaders;
		
		static GLManager& Get();

		GLManager(GLManager&) = delete;
		GLManager(const GLManager&) = delete;
		GLManager& operator=(GLManager&) = delete;
		GLManager& operator=(const GLManager&) = delete;
	private:
		GLManager() = default;
	};
}	
