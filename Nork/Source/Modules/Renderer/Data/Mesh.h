#pragma once

#include "Texture.h"

namespace Nork::Renderer::Data
{
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
		std::vector<std::pair<TextureType, TextureData>> textures;
	};

	struct MeshResource
	{
		GLuint vao, vb, ib;
		GLuint textures[textureTypeCount];
		GLsizei idxCount;
	};

	struct Mesh
	{
		Mesh(MeshResource& resource)
		{
			this->vao = resource.vao;
			this->indices = resource.idxCount;
			for (uint8_t i = 0; i < textureTypeCount; i++)
				this->textures[i] = resource.textures[i];
		}

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
}