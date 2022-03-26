#pragma once

#include "Components/Drawable.h"
#include "Modules/Renderer/Model/MeshFactory.h"
#include "Modules/Renderer/Model/Material.h"

namespace Nork {
	class ResourceManager
	{
	public:
		ResourceManager();
		std::vector<std::pair<ResourceRef<Renderer::Mesh>, ResourceRef<Renderer::Material>>> GetModel(const std::string& id);
		ResourceRef<Renderer::Mesh> GetMesh(const std::string& id);
		ResourceRef<Renderer::Texture2D> GetTexture(const std::string& id);
		ResourceRef<Renderer::Material> GetMaterial(const std::string& id);

		Renderer::MaterialBufferWrapper materialBuffer;
		Renderer::VertexArrayWrapper vaoWrapper;
		Renderer::MeshFactory meshFactory;
	private:
		void LoadModel(const std::string& id);
		std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> models;
		std::unordered_map<std::string, ResourceRef<Renderer::Mesh>> meshes;
		std::unordered_map<std::string, ResourceRef<Renderer::Material>> materials;

		ResourceRef<Renderer::Texture2D> LoadTexture(const std::string& id);
		std::unordered_map<std::string, ResourceRef<Renderer::Texture2D>> textures;

	};
}