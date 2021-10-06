#pragma once

#include "Modules/Renderer/Data/Mesh.h"
#include "Modules/Renderer/Data/Shader.h"
#include "Modules/Renderer/Data/Texture.h"

#include "Modules/Renderer/Loaders/Loaders.h"
#include "Modules/Renderer/Resource/ResourceCreator.h"
#include "Modules/Renderer/Resource/DefaultResources.h"

namespace Nork::Scene
{
	class ResourceManager
	{
	public:
		const std::vector<Renderer::Data::MeshResource>& GetCube()
		{
			return { Renderer::Resource::DefaultResources::cube };
		}

		const std::vector<Renderer::Data::MeshResource>& GetMeshes(const std::string& src)
		{
			auto searchRes = resources.meshes.find(src);
			if (searchRes != resources.meshes.end())
			{
				return searchRes._Ptr->_Myval.second;
			}

			try
			{
				rawDatas.meshes[src] = Renderer::Loaders::LoadModel(src);
				auto& meshesRaw = rawDatas.meshes[src];
				for (size_t i = 0; i < meshesRaw.size(); i++)
				{
					resources.meshes[src].push_back(Renderer::Resource::CreateMesh(meshesRaw[i]));
				}
				return resources.meshes[src];
			}
			catch (...)
			{
				throw;
				// catch renderer exceptions
			}
		}

		const Renderer::Data::TextureResource& GetTexture(const std::string& src)
		{
			auto searchRes = resources.textures.find(src);
			if (searchRes != resources.textures.end())
			{
				return searchRes._Ptr->_Myval.second;
			}

			try
			{
				rawDatas.textures[src] = Renderer::Loaders::LoadImage(src);
				auto& textureRaw = rawDatas.textures[src];
				resources.textures[src] = Renderer::Resource::CreateTexture(textureRaw);
				return resources.textures[src];
			}
			catch (...)
			{
				throw;
				// catch renderer exceptions
			}
		}

		const Renderer::Data::ShaderResource& GetShader(const std::string& src)
		{
			auto searchRes = resources.shaders.find(src);
			if (searchRes != resources.shaders.end())
			{
				return searchRes._Ptr->_Myval.second;
			}

			try
			{
				rawDatas.shaders[src] = Renderer::Loaders::LoadShader(src);
				auto& shaderRaw = rawDatas.shaders[src];
				resources.shaders[src] = Renderer::Resource::CreateShader(shaderRaw);
				return resources.shaders[src];
			}
			catch (...)
			{
				throw;
				// catch renderer exceptions
			}
		}
	private:
		template<typename T>
		using Map = std::unordered_map<std::string, T>;
		struct RawDataStorage
		{
			Map<std::vector<Renderer::Data::MeshData>> meshes;
			Map<Renderer::Data::ShaderData> shaders;
			Map<Renderer::Data::TextureData> textures;
		};
		struct ResourceStorage
		{
			Map<std::vector<Renderer::Data::MeshResource>> meshes;
			Map<Renderer::Data::ShaderResource> shaders;
			Map<Renderer::Data::TextureResource> textures;
		};
	private:
		ResourceStorage resources;
		RawDataStorage rawDatas;
	};
}

