
#include "ResourceManager.h"
#include "Modules/Renderer/LoadUtils.h"
#include "Modules/Renderer/Objects/Texture/TextureBuilder.h"

namespace Nork {
	ResourceRef<Renderer::Texture2D> ResourceManager::GetTexture(const std::string& id)
	{
		auto opt = textures.find(id);
		if (opt == textures.end() || opt->second.expired())
		{
			auto shared = LoadTexture(id);
			textures[id] = shared;
			return shared;
		}
		return textures[id].lock();
	}
	std::shared_ptr<Resource<std::vector<Renderer::Mesh>>> ResourceManager::LoadMeshes(const std::string& id)
	{
		if (id == "")
		{
			return std::make_shared<Resource<std::vector<Renderer::Mesh>>>(id, std::make_shared<std::vector<Renderer::Mesh>>(std::vector<Renderer::Mesh> {Renderer::Mesh::Cube()}));
		}
		auto meshDatas = Renderer::LoadUtils::LoadModel(id);
		auto shared = std::make_shared<std::vector<Renderer::Mesh>>();
		for (auto& meshData : meshDatas)
		{
			if (meshData.indices.size() > 0)
				shared->push_back(Renderer::Mesh(meshData.vertices, meshData.indices));
			else
				shared->push_back(Renderer::Mesh(meshData.vertices));
			for (auto& pair : meshData.textures)
			{
				shared->back().SetTexture(GetTexture(pair.second)->object, pair.first);
			}
		}
		Logger::Info("Loaded model: ", id);
		return std::make_shared<Resource<std::vector<Renderer::Mesh>>>(id, shared);
	}
	std::shared_ptr<Resource<Renderer::Texture2D>> ResourceManager::LoadTexture(const std::string& id)
	{
		using namespace Renderer;
		auto image = LoadUtils::LoadImage(id);
		auto tex = TextureBuilder()
			.Attributes(TextureAttributes{ .width = image.width, .height = image.height, .format = image.format })
			.Params(TextureParams::Tex2DParams())
			.Create2DWithData(image.data.data());
		Logger::Info("Loaded texture: ", id);
		return std::make_shared<Resource<Renderer::Texture2D>>(id, tex);
	}
}