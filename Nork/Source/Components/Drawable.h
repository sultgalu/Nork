#pragma once

#include "Modules/Renderer/Model/Model.h"

namespace Nork {


	template<class T>
	struct Resource
	{
		Resource(const std::string& id, std::shared_ptr<T> object)
			: id(id), object(object)
		{}
		std::string id;
		// T& operator->()
		// {
		// 	return *object.get();
		// }
		// T& operator*()
		// {
		// 	return *object;
		// }
	//private:
		std::shared_ptr<T> object;
	};

	template<class T>
	struct ResourceRef
	{
		std::shared_ptr<Resource<T>> resource = nullptr;
	};

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
				auto resRef = ResourceRef<std::vector<Renderer::Mesh>>{ .resource = shared };
				return resRef;
			}
			return ResourceRef<std::vector<Renderer::Mesh>>{.resource = meshes[id].lock() };
		}

		std::shared_ptr<Resource<std::vector<Renderer::Mesh>>> LoadMeshes(const std::string& id)
		{
			return std::make_shared<Resource<std::vector<Renderer::Mesh>>>(id, std::make_shared<std::vector<Renderer::Mesh>>(std::vector<Renderer::Mesh> {Renderer::Mesh::Cube()}));
		}
		std::unordered_map<std::string, std::weak_ptr<Resource<std::vector<Renderer::Mesh>>>> meshes;
	};

	using MeshResourceRef = ResourceRef<std::vector<Renderer::Mesh>>;
}

namespace Nork::Components
{

	struct Drawable
	{
		MeshResourceRef resource;
		Renderer::Model model = Renderer::Model::Cube();
	};
}