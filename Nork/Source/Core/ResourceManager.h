#pragma once

#include "Components/Drawable.h"

namespace Nork {
	class ResourceManager
	{
	public:
		ResourceRef<std::vector<std::pair<Renderer::Mesh, Renderer::Material>>> GetMeshes(const std::string& id);
		ResourceRef<Renderer::Texture2D> GetTexture(const std::string& id);
		ResourceRef<Renderer::Material> GetMaterial(const std::string& id);
	private:
		ResourceRef<std::vector<std::pair<Renderer::Mesh, Renderer::Material>>> LoadMeshes(const std::string& id);
		std::unordered_map<std::string, ResourceRef<std::vector<std::pair<Renderer::Mesh, Renderer::Material>>>> meshes;

		ResourceRef<Renderer::Texture2D> LoadTexture(const std::string& id);
		std::unordered_map<std::string, std::shared_ptr<Resource<Renderer::Texture2D>>> textures;

		ResourceRef<Renderer::Material> LoadMaterial(const std::string& id);
		std::unordered_map<std::string, std::shared_ptr<Resource<Renderer::Material>>> materials;
	};
}