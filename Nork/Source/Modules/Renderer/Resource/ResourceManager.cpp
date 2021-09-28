#include "pch.h"
#include "ResourceManager.h"
#include "DefaultResources.h"

namespace Nork::Renderer::Resource
{
	using namespace Data;
    MeshResource CreateMesh(MeshData& data)
    {
		MeshResource resource{};

		resource.vao = Utils::VAO::Builder()
			.AddBuffer(&resource.vb, GL_ARRAY_BUFFER, GL_STATIC_DRAW, (int)data.vertices.size() * sizeof(Vertex), data.vertices.data())
			.SetAttribs(std::vector<int>{3, 3, 2, 3, 3 })
			.AddBuffer(&resource.ib, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, (int)data.indices.size() * sizeof(unsigned int), data.indices.data())
			.GetVertexArrayBuffer();

		resource.idxCount = (GLsizei)data.indices.size();

		for (size_t i = 0; i < textureTypeCount; i++)
			resource.textures[i] = DefaultResources::textures[i];

		for (auto& tex : data.textures)
		{
			auto texRes = CreateTexture(tex.second);
			resource.textures[static_cast<uint8_t>(tex.first)] = texRes.id;
		}

		return resource;
    }

    void DeleteMesh(MeshResource& resource)
    {
		GLuint bufs[] = { resource.vb, resource.ib };
		glDeleteBuffers(2, bufs);
		glDeleteVertexArrays(1, &resource.vao);
		std::vector<unsigned int> toDelTexs;
		for (size_t i = 0; i < textureTypeCount; i++)
		{
			if (resource.textures[i] != DefaultResources::textures[i])
				toDelTexs.push_back(resource.textures[i]);
		}
		glDeleteTextures((int)toDelTexs.size(), toDelTexs.data());
    }
}