#pragma once

#include "Modules/Renderer/Model/Mesh.h"
#include "Modules/Renderer/Model/Material.h"

namespace Nork {
	template<class T>
	struct Resource
	{
		Resource(const std::string& id, std::shared_ptr<T> object)
			: id(id), object(object)
		{}
		std::string id;
		std::shared_ptr<T> object;
	};

	template<class T>
	using ResourceRef = std::shared_ptr<Resource<T>>;

	using MeshResourceRef = ResourceRef<std::vector<std::pair<Renderer::Mesh, Renderer::Material>>>;
}

namespace Nork::Components
{
	struct Mesh
	{
		ResourceRef<Renderer::Mesh> mesh;
		ResourceRef<Renderer::Material> material;
	};

	struct Drawable
	{
		std::vector<Mesh> meshes;
		std::shared_ptr<size_t> modelMatrix;
	};
}