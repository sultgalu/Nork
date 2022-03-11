#include "DrawUtils.h"
#include "Objects/VertexArray/VertexArray.h"

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

	static VertexArray GetQuadVao()
	{
		VertexArray vao = VertexArray().Create().Bind();
		auto verts = DrawUtils::GetQuadVertices();
		auto inds = DrawUtils::GetQuadIndices();
		vao.GetVBO().Create().Bind(BufferTarget::Vertex).Allocate(verts.size() * sizeof(float), verts.data());
		vao.GetIBO().Create().Bind(BufferTarget::Index).Allocate(inds.size() * sizeof(GLuint), inds.data());
		vao.SetAttribs({2, 2});
		return vao;
	}

	static VertexArray GetCubeVao()
	{
		VertexArray vao = VertexArray().Create().Bind();
		auto verts = DrawUtils::GetCubeVertices();
		auto inds = DrawUtils::GetCubeIndices();
		vao.GetVBO().Create().Bind(BufferTarget::Vertex).Allocate(verts.size() * sizeof(float), verts.data());
		vao.GetIBO().Create().Bind(BufferTarget::Index).Allocate(inds.size() * sizeof(GLuint), inds.data());
		vao.SetAttribs({ 3 });
		return vao;
	}

	void DrawUtils::DrawQuad()
	{
		static VertexArray quadVao = GetQuadVao();
		quadVao.Bind().DrawIndexed();
		//glBindVertexArray(quadVao);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	void DrawUtils::DrawCube()
	{
		static VertexArray cubeVao = GetCubeVao();
		cubeVao.Bind().DrawIndexed();
	}
}