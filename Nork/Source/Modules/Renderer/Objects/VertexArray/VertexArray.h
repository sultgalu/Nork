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
		VertexArray& Create()
		{
			Logger::Info("Creating vertex array ", handle, ".");
			glGenVertexArrays(1, &handle);
			return *this;
		}
		void Destroy()
		{
			Logger::Info("Deleting vertex array ", handle, ".");
			glDeleteVertexArrays(1, &handle);
		}
		VertexArray& Bind()
		{
			glBindVertexArray(handle);
			return *this;
		}
		Buffer& GetVBO() { return vbo; }
		Buffer& GetIBO() { return ibo; }
		void Draw(DrawMode mode = DrawMode::Triangles)
		{
			glDrawArrays(std::to_underlying(mode), 0, vbo.GetSize() / stride);
		}
		void DrawIndexed(std::span<uint32_t> indices, DrawMode mode = DrawMode::Triangles)
		{
			glDrawElements(std::to_underlying(mode), indices.size(), GL_UNSIGNED_INT, indices.data());
		}
		void DrawIndexed(DrawMode mode = DrawMode::Triangles)
		{
			glDrawElements(std::to_underlying(mode), ibo.GetSize() / sizeof(GLuint), GL_UNSIGNED_INT, 0);
		}
		VertexArray& SetAttribs(std::vector<int> attrLens)
		{
			this->attrLens = attrLens;
			Bind();
			vbo.Bind(BufferTarget::Vertex);

			stride = 0;
			for (int i = 0; i < attrLens.size(); i++)
				stride += attrLens[i];
			stride *= sizeof(float);

			int offset = 0;
			for (int i = 0; i < attrLens.size(); i++)
			{
				glVertexAttribPointer(i, attrLens[i], GL_FLOAT, false, stride, (void*)(offset * sizeof(float)));
				glEnableVertexAttribArray(i);
				offset += attrLens[i];
			}

			return *this;
		}
		bool HasIbo()
		{
			return ibo.IsCreated();
		}
		bool HasVbo()
		{
			return vbo.IsCreated();
		}
	protected:
		std::vector<int> attrLens;
		int stride;
		Buffer vbo, ibo;
	};
}

