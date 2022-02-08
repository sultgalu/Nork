#pragma once

#include "../Objects/VertexArray/VertexArray.h"
#include "../Objects/Texture/Texture.h"

namespace Nork::Renderer2 {
	enum class TextureMapType : uint8_t
	{
		Diffuse = 0, Normal, Roughness, Reflection, COUNT
	};
	using MeshTextureMaps = std::array<Texture2D, std::to_underlying(TextureMapType::COUNT)>;

	struct Vertex
	{
		glm::vec3 position, normal;
		glm::vec2 texCoords;
		glm::vec3 tangent, biTangent;
	};

	struct Mesh
	{
		MeshTextureMaps textureMaps;
		VertexArrayIndexed vao;

		inline void BindTextures()
		{
			for (int i = 0; i < textureMaps.size(); i++)
			{
				textureMaps[i].Bind(i);
			}
		}
		inline void Draw()
		{
			vao.DrawIndexed();
		}
	};	
}