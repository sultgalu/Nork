#pragma once

#include "Texture.h"

namespace Nork::Renderer::Data
{
	enum class TextureUse : uint8_t
	{
		Diffuse = 0, Normal, Roughness, Reflection, COUNT
	};
	constinit const uint8_t textureTypeCount = static_cast<uint8_t>(TextureUse::COUNT);

	struct Vertex
	{
		glm::vec3 Position, Normal;
		glm::vec2 TexCoords;
		glm::vec3 tangent, biTangent;
	};

	struct MeshData
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<std::pair<TextureUse, TextureData>> textures;
	};

	struct MeshResource
	{
		GLuint vao, vb, ib;
		TextureResource textures[textureTypeCount];
		GLsizei idxCount;
	};

	struct Mesh
	{
		Mesh(const MeshResource& resource)
		{
			this->vao = resource.vao;
			this->indices = resource.idxCount;
			for (uint8_t i = 0; i < textureTypeCount; i++)
				this->textures[i] = resource.textures[i].id;
		}

		Mesh() = default;

		GLuint vao;
		GLsizei indices;
		GLuint textures[textureTypeCount];

		inline void BindTextures()
		{
			for (int i = 0; i < textureTypeCount; i++)
			{
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, textures[i]);
			}
		}
		inline void Draw()
		{
			glBindVertexArray(vao);
			glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, 0);
		}
	};

	typedef std::pair<std::vector<Mesh>, glm::mat4> Model;
}