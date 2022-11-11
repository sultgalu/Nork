#include "DrawQuadCommand.h"
#include "../../Objects/Buffer/BufferBuilder.h"
#include "../../Objects/VertexArray/VertexArrayBuilder.h"

namespace Nork::Renderer {
	static std::vector<float> GetVertices()
	{
		return {
				-1.0f, -1.0f, 0.0f, 0.0f,
				1.0f, -1.0f, 1.0f, 0.0f,
				1.0f, 1.0f, 1.0f, 1.0f,
				-1.0f, 1.0f, 0.0f, 1.0f,
		};
	}
	static std::vector<GLuint> GetIndices()
	{
		return {
				0, 1, 3,
				3, 1, 2,
		};
	}

	DrawQuadCommand::DrawQuadCommand()
	{
		auto verts = GetVertices();
		auto inds = GetIndices();
		vao = VertexArrayBuilder()
			.VBO(BufferBuilder()
				.Target(BufferTarget::Vertex)
				.Flags(BufferStorageFlags::None)
				.Data(verts.data(), verts.size() * sizeof(float))
				.Create())
			.IBO(BufferBuilder()
				.Target(BufferTarget::Index)
				.Flags(BufferStorageFlags::None)
				.Data(inds.data(), inds.size() * sizeof(GLuint))
				.Create())
			.Attributes({ 2, 2 })
			.Create();
	}

	void DrawQuadCommand::operator()() const
	{
		vao->Bind().DrawIndexed();
	}
}
