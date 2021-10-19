#include "pch.h"
#include "ResourceCreator.h"
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
			resource.textures[static_cast<uint8_t>(tex.first)] = texRes;
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
			if (resource.textures[i].id != DefaultResources::textures[i].id)
				toDelTexs.push_back(resource.textures[i].id);
		}
		glDeleteTextures((int)toDelTexs.size(), toDelTexs.data());
    }

	ShaderResource CreateShader(ShaderData& data)
	{ 
		auto resource = ShaderResource{ .program = Utils::Shader::GetProgramFromSource(data.source) };
		if (resource.program == 0)
		{
			Logger::Error("Failed to create shader.");
		}
		return resource;
	}

	TextureResource CreateCubemap(std::array<TextureData, 6>& datas, Utils::Texture::TextureParams params)
	{
		std::array<void*, 6> pointers;
		for (size_t i = 0; i < 6; i++)
		{
			pointers[i] = datas[i].data.size() == 0 ? nullptr : datas[i].data.data();
		}

		using namespace Utils::Texture;
		auto resource = TextureResource{ .id = Utils::Texture::Create<TextureType::Cube>(datas[0].width, datas[0].height, datas[0].format, pointers.data(), params)};
		if (resource.id == 0)
		{
			MetaLogger().Error("Failed to create Cubemap Texture.");
		}
		return resource;
	}
	TextureResource CreateTexture(TextureData& data, Utils::Texture::TextureParams params)
	{
		auto resource = TextureResource{ .id = Utils::Texture::Create(data.width, data.height, data.format, data.data.size() == 0 ? nullptr : data.data.data(), params) };
		if (resource.id == 0)
		{
			MetaLogger().Error("Failed to create Texture.");
		}
		return resource;
	}
}