#include "DrawCubeCommand.h"
#include "../../Objects/Buffer/BufferBuilder.h"
#include "../../Objects/VertexArray/VertexArrayBuilder.h"

namespace Nork::Renderer {
	static std::vector<float> GetVertices()
	{
		return std::vector<float> {
			-1.0f, -1.0f, -1.0f,
				1.0f, -1.0f, -1.0f,
				1.0f, 1.0f, -1.0f,
				-1.0f, 1.0f, -1.0f,

				-1.0f, -1.0f, 1.0f,
				1.0f, -1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,
				-1.0f, 1.0f, 1.0f,
		};
	}
	static std::vector<GLuint> GetIndices()
	{
		std::vector<GLuint> indices(36);
		unsigned int offset = 0, i = 0;

		{
			//CLOCK-WISE

			// Face #1
			indices[i++] = offset + 0;
			indices[i++] = offset + 1;
			indices[i++] = offset + 2;

			indices[i++] = offset + 2;
			indices[i++] = offset + 3;
			indices[i++] = offset + 0;

			// Face #2
			indices[i++] = offset + 2;
			indices[i++] = offset + 1;
			indices[i++] = offset + 5;

			indices[i++] = offset + 5;
			indices[i++] = offset + 6;
			indices[i++] = offset + 2;

			// Face #3
			indices[i++] = offset + 4;
			indices[i++] = offset + 0;
			indices[i++] = offset + 3;

			indices[i++] = offset + 3;
			indices[i++] = offset + 7;
			indices[i++] = offset + 4;

			// Face #4
			indices[i++] = offset + 1;
			indices[i++] = offset + 0;
			indices[i++] = offset + 4;

			indices[i++] = offset + 4;
			indices[i++] = offset + 5;
			indices[i++] = offset + 1;

			// Face #5
			indices[i++] = offset + 7;
			indices[i++] = offset + 3;
			indices[i++] = offset + 2;

			indices[i++] = offset + 2;
			indices[i++] = offset + 6;
			indices[i++] = offset + 7;

			// Face #6
			indices[i++] = offset + 6;
			indices[i++] = offset + 5;
			indices[i++] = offset + 4;

			indices[i++] = offset + 4;
			indices[i++] = offset + 7;
			indices[i++] = offset + 6;
		}

		return indices;
	}

	DrawCubeCommand::DrawCubeCommand()
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
			.Attributes({ 3 })
			.Create();
	}

	void DrawCubeCommand::operator()() const
	{
		vao->Bind().DrawIndexed();
	}
}
