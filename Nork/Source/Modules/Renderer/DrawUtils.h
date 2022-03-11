#pragma once

namespace Nork::Renderer {
	class DrawUtils
	{
	public:
		static std::vector<float> GetQuadVertices();
		static std::vector<GLuint> GetQuadIndices();
		static std::vector<float> GetCubeVertices();
		static std::vector<GLuint> GetCubeIndices();
		static void DrawQuad();
		static void DrawCube();
	};
}