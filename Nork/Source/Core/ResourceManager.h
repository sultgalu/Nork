#pragma once

#include "Components/Drawable.h"

namespace Nork {
	class ResourceManager
	{
	public:
		ResourceRef<std::vector<Renderer::Mesh>> GetMeshes(const std::string& id)
		{
			auto opt = meshes.find(id);
			if (opt == meshes.end() || opt->second.expired())
			{
				auto shared = LoadMeshes(id);
				meshes[id] = shared;
				return shared;
			}
			return meshes[id].lock();
		}
		ResourceRef<Renderer::Texture2D> GetTexture(const std::string& id);
	private:
		std::shared_ptr<Resource<std::vector<Renderer::Mesh>>> LoadMeshes(const std::string& id);
		std::unordered_map<std::string, std::weak_ptr<Resource<std::vector<Renderer::Mesh>>>> meshes;

		std::shared_ptr<Resource<Renderer::Texture2D>> LoadTexture(const std::string& id);
		std::unordered_map<std::string, std::weak_ptr<Resource<Renderer::Texture2D>>> textures;
	};
}