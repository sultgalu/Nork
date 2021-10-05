#include "pch.h"
#include "DefaultResources.h"
#include "ResourceManager.h"
#include "../Utils.h"

namespace Nork::Renderer::Resource
{
	static void InitDefaultTextures()
	{
		using namespace Utils::Texture;
		float diff[]{ 1.0f, 1.0f, 1.0f, 1.0f };
		float norm[]{ 0.5f, 0.5f, 1.0f };
		float refl[]{ 0.0f };
		float rough[]{ 1.0f };
		DefaultResources::textures[static_cast<uint8_t>(Data::TextureType::Diffuse)] = Data::TextureResource{ .id = Create2D(1, 1, Format::RGBA, 1, false, diff) };
		DefaultResources::textures[static_cast<uint8_t>(Data::TextureType::Normal)] = Data::TextureResource{ .id = Create2D(1, 1, Format::RGB, 1, false, norm) };
		DefaultResources::textures[static_cast<uint8_t>(Data::TextureType::Reflection)] = Data::TextureResource{ .id = Create2D(1, 1, Format::R8, 1, false, refl) };
		DefaultResources::textures[static_cast<uint8_t>(Data::TextureType::Roughness)] = Data::TextureResource{ .id = Create2D(1, 1, Format::R8, 1, false, rough) };
	}

	static void InitCube()
	{
		auto positions = Utils::Mesh::GetCubeVertexPositions();
		auto texCoords = Utils::Mesh::GetCubeVertexTexCoords();
		auto normals = Utils::Mesh::GetCubeVertexNormals();
		auto tangents = Utils::Mesh::GetCubeVertexTangents();
		auto bitangents = Utils::Mesh::GetCubeVertexBitangents();

		Data::MeshData meshData;
		for (int i = 0; i < positions.size() / 3; i++)
		{
			Data::Vertex vertex;
			vertex.Position = glm::vec3(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
			vertex.Normal = glm::vec3(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
			vertex.tangent = glm::vec3(tangents[i * 3], tangents[i * 3 + 1], tangents[i * 3 + 2]);
			vertex.biTangent = glm::vec3(bitangents[i * 3], bitangents[i * 3 + 1], bitangents[i * 3 + 2]);
			vertex.TexCoords = glm::vec2(texCoords[i * 2], texCoords[i * 2 + 1]);

			meshData.vertices.push_back(vertex);
		}
		meshData.indices = Utils::Mesh::GetCubeIndices();
		// leaving textures empty, resourceMan sets the default ones.

		DefaultResources::cube = Renderer::Resource::CreateMesh(meshData);
	}

	void DefaultResources::Init()
	{
		InitDefaultTextures();
		InitCube();
	}
	void DefaultResources::Free()
	{
	}
}


