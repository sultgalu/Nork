#pragma once

#include "../Objects/GLManager.h"
#include "../Objects/VertexArray/VertexArray.h"
#include "../Objects/Texture/Texture.h"
#include "../Objects/GLManager.h"

namespace Nork::Renderer {
	enum class TextureMapType : uint8_t
	{
		Diffuse = 0, Normal, Roughness, Reflection, COUNT
	};

	struct Vertex
	{
		glm::vec3 position, normal;
		glm::vec2 texCoords;
		glm::vec3 tangent, biTangent;
	};

	class Mesh
	{
	public:
		Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);
		Mesh& SetTexture(std::shared_ptr<Texture2D> texture, TextureMapType type)
		{
			textureMaps[std::to_underlying(type)] = texture;
			return *this;
		}
		Mesh& DiffuseMap(std::shared_ptr<Texture2D> texture)
		{
			return SetTexture(texture, TextureMapType::Diffuse);
		}
		Mesh& NormalMap(std::shared_ptr<Texture2D> texture)
		{
			return SetTexture(texture, TextureMapType::Normal);
		}
		Mesh& ReflectionMap(std::shared_ptr<Texture2D> texture)
		{
			return SetTexture(texture, TextureMapType::Reflection);
		}
		Mesh& RoughnessMap(std::shared_ptr<Texture2D> texture)
		{
			return SetTexture(texture, TextureMapType::Roughness);
		}

		std::shared_ptr<Texture2D> DiffuseMap()
		{
			return textureMaps[std::to_underlying(TextureMapType::Diffuse)];
		}
		std::shared_ptr<Texture2D> NormalMap()
		{
			return textureMaps[std::to_underlying(TextureMapType::Normal)];
		}
		std::shared_ptr<Texture2D> ReflectionMap()
		{
			return textureMaps[std::to_underlying(TextureMapType::Reflection)];
		}
		std::shared_ptr<Texture2D> RoughnessMap()
		{
			return textureMaps[std::to_underlying(TextureMapType::Roughness)];
		}

		Mesh& BindTextures()
		{
			for (int i = 0; i < textureMaps.size(); i++)
			{
				textureMaps[i]->Bind(i);
			}
			return *this;
		}
		void Draw()
		{
			vao->Bind().DrawIndexed();
		}
		static Mesh Cube();
	private:
		static std::array<std::shared_ptr<Texture2D>, std::to_underlying(TextureMapType::COUNT)> GetDefaultTextureMaps();
	private:
		std::array<std::shared_ptr<Texture2D>, std::to_underlying(TextureMapType::COUNT)> textureMaps;
		std::shared_ptr<VertexArray> vao;
	};	
}