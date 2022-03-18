#pragma once

#include "../GLObject.h"
#include "../Buffer/Buffer.h"

namespace Nork::Renderer {
	enum class DrawMode: GLenum
	{
		Triangles = GL_TRIANGLES, 
		Lines = GL_LINES,
		Points = GL_POINTS,
	};

	class VertexArray: public GLObject
	{
	public:
		~VertexArray()
		{
			Logger::Info("Deleting vertex array ", handle, ".");
			glDeleteVertexArrays(1, &handle);
		}
		VertexArray& Bind()
		{
			glBindVertexArray(handle);
			return *this;
		}
		std::shared_ptr<Buffer> GetVBO() { return vbo; }
		std::shared_ptr<Buffer> GetIBO() { return ibo; }
		void Draw(DrawMode mode = DrawMode::Triangles)
		{
			glDrawArrays(std::to_underlying(mode), 0, vbo->GetSize() / stride);
		}
		void DrawIndexed(std::span<uint32_t> indices, DrawMode mode = DrawMode::Triangles)
		{
			glDrawElements(std::to_underlying(mode), indices.size(), GL_UNSIGNED_INT, indices.data());
		}
		void DrawIndexed(DrawMode mode = DrawMode::Triangles)
		{
			ibo->Bind();
			glDrawElements(std::to_underlying(mode), ibo->GetSize() / sizeof(GLuint), GL_UNSIGNED_INT, 0);
		}
		void DrawIndexedInstanced(uint32_t count, DrawMode mode = DrawMode::Triangles)
		{
			ibo->Bind();
			glDrawElementsInstanced(std::to_underlying(mode), ibo->GetSize() / sizeof(GLuint), GL_UNSIGNED_INT, 0, count);
		}
		bool HasIbo()
		{
			return ibo != nullptr;
		}
		VertexArray(GLuint handle, std::vector<int> attrLens, int stride, std::shared_ptr<Buffer> vbo, std::shared_ptr<Buffer> ibo)
			: VertexArray(handle, attrLens, stride, vbo)
		{
			this->ibo = ibo;
		}
		VertexArray(GLuint handle, std::vector<int> attrLens, int stride, std::shared_ptr<Buffer> vbo)
			: GLObject(handle), attrLens(attrLens), stride(stride), vbo(vbo) 
		{}
	private:
		const std::vector<int> attrLens;
		const int stride;
		std::shared_ptr<Buffer> vbo, ibo = nullptr;
	private:
		GLenum GetIdentifier() override
		{
			return GL_VERTEX_ARRAY;
		}
	};
}

