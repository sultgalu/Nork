#pragma once

#include "../Objects/VertexArray/VertexArray.h"
#include "../Objects/Texture/Texture.h"

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

	struct Mesh
	{
		Mesh& CreateVAO(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices)
		{
			vao.GetVBO().Create().Bind(BufferTarget::Vertex).Allocate(sizeof(Vertex) * vertices.size(), vertices.data());
			vao.Create().Bind().SetAttribs(std::vector<int>{3, 3, 2, 3, 3 });
			vao.GetIBO().Create().Bind(BufferTarget::Index).Allocate(sizeof(unsigned int) * indices.size(), indices.data());
			return *this;
		}
		Mesh& SetTextureMaps(std::vector<std::pair<TextureMapType, Texture2D>> textures = {})
		{
			textureMaps = GetDefaultTextureMaps();
			for (auto& tex : textures)
			{
				textureMaps[std::to_underlying(tex.first)] = tex.second;
			}
			return *this;
		}
		std::array<Texture2D, std::to_underlying(TextureMapType::COUNT)> textureMaps;
		VertexArray vao;

		inline void BindTextures()
		{
			for (int i = 0; i < textureMaps.size(); i++)
			{
				textureMaps[i].Bind(i);
			}
		}
		inline void Draw()
		{
			vao.Bind().DrawIndexed();
		}
	private:
		static Texture2D CreateDefaultDiffuse()
		{
			float data[]{ 1.0f, 1.0f, 1.0f, 1.0f };
			return Texture2D().Create().Bind().SetParams().SetData(TextureAttributes{ .width = 1, .height = 1, .format = TextureFormat::RGBA32F }, data);
		}
		static Texture2D CreateDefaultNormal()
		{
			float data[]{ 0.5f, 0.5f, 1.0f };
			return Texture2D().Create().Bind().SetParams().SetData(TextureAttributes{ .width = 1, .height = 1, .format = TextureFormat::RGB32F }, data);
		}
		static Texture2D CreateDefaultReflective()
		{
			float data[]{ 0.5f };
			return Texture2D().Create().Bind().SetParams().SetData(TextureAttributes{ .width = 1, .height = 1, .format = TextureFormat::R32F }, data);
		}
		static Texture2D CreateDefaultRoughness()
		{
			float data[]{ 0.5f };
			return Texture2D().Create().Bind().SetParams().SetData(TextureAttributes{ .width = 1, .height = 1, .format = TextureFormat::R32F }, data);
		}
		static std::array<Texture2D, std::to_underlying(TextureMapType::COUNT)> GetDefaultTextureMaps()
		{
			static auto diff = CreateDefaultDiffuse();
			static auto norm = CreateDefaultNormal();
			static auto refl = CreateDefaultReflective();
			static auto rough = CreateDefaultRoughness();
			return { diff, norm, refl, rough };
		}
	};	
}