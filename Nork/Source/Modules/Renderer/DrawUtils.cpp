#include "DrawUtils.h"
#include "Objects/GLManager.h"

namespace Nork::Renderer {

	std::vector<float> DrawUtils::GetQuadVertices()
	{
		return {
				-1.0f, -1.0f, 0.0f, 0.0f,
				1.0f, -1.0f, 1.0f, 0.0f,
				1.0f, 1.0f, 1.0f, 1.0f,
				-1.0f, 1.0f, 0.0f, 1.0f,
		};
	}

	std::vector<GLuint> DrawUtils::GetQuadIndices()
	{
		return {
				0, 1, 3,
				3, 1, 2,
		};
	}

	std::vector<float> DrawUtils::GetCubeVertices()
	{
		return std::vector<float> {
			   -1.0f, -1.0f, -1.0f,
				1.0f, -1.0f, -1.0f,
				1.0f,  1.0f, -1.0f,
			   -1.0f,  1.0f, -1.0f,

			   -1.0f, -1.0f,  1.0f,
				1.0f, -1.0f,  1.0f,
				1.0f,  1.0f,  1.0f,
			   -1.0f,  1.0f,  1.0f,
		};
	}

	std::vector<GLuint> DrawUtils::GetCubeIndices()
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

	static std::shared_ptr<VertexArray> GetQuadVao()
	{
		auto verts = DrawUtils::GetQuadVertices();
		auto inds = DrawUtils::GetQuadIndices();
		auto vbo = BufferBuilder().Target(BufferTarget::Vertex).Usage(BufferUsage::StaticDraw).Data(verts.data(), verts.size() * sizeof(float)).Create();
		auto ibo = BufferBuilder().Target(BufferTarget::Index).Usage(BufferUsage::StaticDraw).Data(inds.data(), inds.size() * sizeof(GLuint)).Create();
		auto vao = VertexArrayBuilder().VBO(vbo).IBO(ibo).Attributes({ 2, 2 }).Create();
		return vao;
	}

	static std::shared_ptr<VertexArray> GetCubeVao()
	{
		auto verts = DrawUtils::GetCubeVertices();
		auto inds = DrawUtils::GetCubeIndices();
		auto vbo = BufferBuilder().Target(BufferTarget::Vertex).Usage(BufferUsage::StaticDraw).Data(verts.data(), verts.size() * sizeof(float)).Create();
		auto ibo = BufferBuilder().Target(BufferTarget::Index).Usage(BufferUsage::StaticDraw).Data(inds.data(), inds.size() * sizeof(GLuint)).Create();
		auto vao = VertexArrayBuilder().VBO(vbo).IBO(ibo).Attributes({ 3 }).Create();
		return vao;
	}

	void DrawUtils::DrawQuad()
	{
		static auto quadVao = GetQuadVao();
		quadVao->Bind().DrawIndexed();
		// glBindVertexArray(quadVao);
		// glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	void DrawUtils::DrawCube()
	{
		static auto cubeVao = GetCubeVao();
		cubeVao->Bind().DrawIndexed();
	}
}