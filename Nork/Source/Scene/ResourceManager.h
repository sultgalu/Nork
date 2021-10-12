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
		const std::vector<Renderer::Data::MeshResource> GetCube()
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

			rawDatas.meshes[src] = Renderer::Loaders::LoadModel(src);
			auto& meshesRaw = rawDatas.meshes[src];
			for (size_t i = 0; i < meshesRaw.size(); i++)
			{
				resources.meshes[src].push_back(Renderer::Resource::CreateMesh(meshesRaw[i]));
			}

			references.meshGroups[src]++;
			Logger::Info("Loaded model from ", src,
				".\n\tMeshes: ", meshesRaw.size(),
				"\n\tFirst:",
				"\n\t   Vertices:  ", meshesRaw[0].vertices.size(),
				"\n\t   Indices:   ", meshesRaw[0].indices.size(),
				"\n\t   Textures:  ", meshesRaw[0].textures.size());
			return resources.meshes[src];
		}

		const Renderer::Data::TextureResource& GetTexture(const std::string& src)
		{
			auto searchRes = resources.textures.find(src);
			if (searchRes != resources.textures.end())
			{
				return searchRes._Ptr->_Myval.second;
			}

			rawDatas.textures[src] = Renderer::Loaders::LoadImage(src);
			auto& textureRaw = rawDatas.textures[src];
			resources.textures[src] = Renderer::Resource::CreateTexture(textureRaw);

			references.textures[src]++; 
			Logger::Info("Loaded Texture from ", src, "\n\t", std::format("{}:{} {} bit", textureRaw.width, textureRaw.height, textureRaw.channels));
			return resources.textures[src];			
		}

		const Renderer::Data::ShaderResource& GetShader(const std::string& src)
		{
			auto searchRes = resources.shaders.find(src);
			if (searchRes != resources.shaders.end())
			{
				return searchRes._Ptr->_Myval.second;
			}

			rawDatas.shaders[src] = Renderer::Loaders::LoadShader(src);
			auto& shaderRaw = rawDatas.shaders[src];
			resources.shaders[src] = Renderer::Resource::CreateShader(shaderRaw);

			references.shaders[src]++;
			Logger::Info("Loaded Shader from ", src);
			return resources.shaders[src];
		}

		void DroppedMeshResource(std::string& src)
		{
			if (DecreaseRefCount(references.meshGroups, src))
			{
				auto& meshes = resources.meshes[src];
				for (size_t i = 0; i < meshes.size(); i++)
				{
					Renderer::Resource::DeleteMesh(meshes[i]);
				}
				resources.meshes.erase(src);
				rawDatas.meshes.erase(src);
			}
		}
		void DroppedTextureResource(std::string& src)
		{
			if (DecreaseRefCount(references.textures, src))
			{
				Renderer::Resource::DeleteTexture(resources.textures[src]);
				resources.textures.erase(src);
				rawDatas.textures.erase(src);
			}
		}
		void DroppedShaderResource(std::string& src)
		{
			if (DecreaseRefCount(references.shaders, src))
			{
				Renderer::Resource::DeleteShader(resources.shaders[src]);
				resources.shaders.erase(src);
				rawDatas.shaders.erase(src);
			}
		}
	private:
		template<typename T>
		using Map = std::unordered_map<std::string, T>;

		inline bool DecreaseRefCount(Map<int32_t>& map, std::string& src)
		{
			if (!map.contains(src))
			{
				MetaLogger().Error(src, " was never contained");
				return false;
			}
			auto& count = map[src];
			count--;
			if (count == 0)
			{
				Logger::Info("Deleting resource \"", src, "\", because no more references");
				return true;
			}
			else if (count < 0) [[unlikely]]
			{
				MetaLogger().Error("Less, than 0 references to \"", src, "\", this should not happen");
			}
			return false;
		}
	private:
		struct RawDatas
		{
			Map<std::vector<Renderer::Data::MeshData>> meshes;
			Map<Renderer::Data::ShaderData> shaders;
			Map<Renderer::Data::TextureData> textures;
		};
		struct Resources
		{
			Map<std::vector<Renderer::Data::MeshResource>> meshes;
			Map<Renderer::Data::ShaderResource> shaders;
			Map<Renderer::Data::TextureResource> textures;
		};
		struct References
		{
			Map<int32_t> meshGroups;
			Map<int32_t> shaders;
			Map<int32_t> textures;
		};
	private:
		Resources resources;
		RawDatas rawDatas;
		References references;
	};
}

