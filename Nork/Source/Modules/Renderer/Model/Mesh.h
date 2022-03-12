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

	struct Mesh
	{
		Mesh& CreateVAO(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices)
		{
			auto vbo = BufferBuilder().Target(BufferTarget::Vertex).Usage(BufferUsage::StaticDraw).Data(vertices.data(), vertices.size() * sizeof(Vertex)).Create();
			auto ibo = BufferBuilder().Target(BufferTarget::Index).Usage(BufferUsage::StaticDraw).Data(indices.data(), indices.size() * sizeof(GLuint)).Create();
			vao = VertexArrayBuilder().VBO(vbo).IBO(ibo).Attributes({ 3, 3, 2, 3, 3 }).Create();
			return *this;
		}
		Mesh& SetTextureMaps(std::vector<std::pair<TextureMapType, std::shared_ptr<Texture2D>>> textures = {})
		{
			textureMaps = GetDefaultTextureMaps();
			for (auto& tex : textures)
			{
				textureMaps[std::to_underlying(tex.first)] = tex.second;
			}
			return *this;
		}
		std::array<std::shared_ptr<Texture2D>, std::to_underlying(TextureMapType::COUNT)> textureMaps;
		std::shared_ptr<VertexArray> vao;

		inline void BindTextures()
		{
			for (int i = 0; i < textureMaps.size(); i++)
			{
				textureMaps[i]->Bind(i);
			}
		}
		inline void Draw()
		{
			vao->Bind().DrawIndexed();
		}
	private:
		static std::shared_ptr<Texture2D> CreateDefaultDiffuse()
		{
			float data[]{ 1.0f, 1.0f, 1.0f, 1.0f };
			return CreateTexture2D(TextureFormat::RGBA32F, data);
		}
		static std::shared_ptr<Texture2D> CreateDefaultNormal()
		{
			float data[]{ 0.5f, 0.5f, 1.0f };
			return CreateTexture2D(TextureFormat::RGB32F, data);
		}
		static std::shared_ptr<Texture2D> CreateDefaultReflective()
		{
			float data[]{ 0.5f };
			return CreateTexture2D(TextureFormat::R32F, data);
		}
		static std::shared_ptr<Texture2D> CreateDefaultRoughness()
		{
			float data[]{ 0.5f };
			return CreateTexture2D(TextureFormat::R32F, data);
		}
		static std::shared_ptr<Texture2D> CreateTexture2D(TextureFormat format, void* data)
		{
			return TextureBuilder()
				.Params(TextureParams::Tex2DParams())
				.Attributes(TextureAttributes{ .width = 1, .height = 1, .format = format })
				.Create2DWithData(data);
		}
		static std::array<std::shared_ptr<Texture2D>, std::to_underlying(TextureMapType::COUNT)> GetDefaultTextureMaps()
		{
			static auto diff = CreateDefaultDiffuse();
			static auto norm = CreateDefaultNormal();
			static auto refl = CreateDefaultReflective();
			static auto rough = CreateDefaultRoughness();
			return { diff, norm, refl, rough };
		}
	};	
}