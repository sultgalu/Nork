#pragma once

#include "../Objects/VertexArray/VertexArray.h"
#include "../Objects/Texture/Texture.h"

namespace Nork::Renderer {
	struct Vertex
	{
		glm::vec3 position, normal;
		glm::vec2 texCoords;
		glm::vec3 tangent;
	};

	class Mesh
	{
	public:
		Mesh(std::vector<Vertex>& vertices);
		Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);
		void Draw() const;
		void DrawInstanced(uint32_t count) const;
		static Mesh Cube();
		std::shared_ptr<VertexArray> vao;
	private:
	};	
}