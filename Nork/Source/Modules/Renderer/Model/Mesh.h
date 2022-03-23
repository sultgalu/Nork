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
		void Draw() const
		{
			if (vao->HasIbo())
				vao->Bind().DrawIndexed();
			else
				vao->Bind().Draw();
		}
		void DrawInstanced(uint32_t count) const
		{
			if (vao->HasIbo())
				vao->Bind().DrawIndexedInstanced(count);
			else
				vao->Bind().DrawInstanced(count);
		}
		static Mesh Cube();
	private:
		std::shared_ptr<VertexArray> vao;
	};	
}