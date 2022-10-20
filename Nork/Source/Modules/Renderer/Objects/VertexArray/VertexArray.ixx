export module Nork.Renderer:VertexArray;

export import :GLObject;
export import :Buffer;

export namespace Nork::Renderer {
	enum class DrawMode: GLenum
	{
		Triangles = GL_TRIANGLES, 
		Lines = GL_LINES,
		Points = GL_POINTS,
	};

	class VertexArray: public GLObject
	{
	public:
		VertexArray(GLuint handle, std::vector<int> attrLens, int stride, std::shared_ptr<Buffer> vbo, std::shared_ptr<Buffer> ibo)
			: VertexArray(handle, attrLens, stride, vbo)
		{
			this->ibo = ibo;
		}
		VertexArray(GLuint handle, std::vector<int> attrLens, int stride, std::shared_ptr<Buffer> vbo)
			: GLObject(handle), attrLens(attrLens), stride(stride), vbo(vbo)
		{}
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
		struct DrawElementsIndirectCommand
		{
			DrawElementsIndirectCommand(uint32_t firstIndex, uint32_t indexCount, uint32_t instanceCount, int baseVertex, uint32_t baseInstance)
				: count(indexCount), instanceCount(instanceCount), firstIndex(firstIndex), baseVertex(baseVertex), baseInstance(baseInstance)
			{}
			uint32_t count; // IBO size (normally)
			uint32_t instanceCount; // instances
			uint32_t firstIndex; // IBO offset
			int baseVertex; // VBO offset
			uint32_t baseInstance = 0;
		};
		void MultiDrawInstanced(const DrawElementsIndirectCommand* commands, uint32_t count, DrawMode mode = DrawMode::Triangles);
		void DrawIndexedInstanced(uint32_t count, DrawMode mode = DrawMode::Triangles)
		{
			ibo->Bind();
			glDrawElementsInstanced(std::to_underlying(mode), ibo->GetSize() / sizeof(GLuint), GL_UNSIGNED_INT, 0, count);
		}
		void DrawInstanced(uint32_t count, DrawMode mode = DrawMode::Triangles)
		{
			glDrawArraysInstanced(std::to_underlying(mode), 0, ibo->GetSize() / sizeof(GLuint), count);
		}
		bool HasIbo()
		{
			return ibo != nullptr;
		}
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

