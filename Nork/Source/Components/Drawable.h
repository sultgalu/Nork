#pragma once

#include "Modules/Renderer/Model/Mesh.h"

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

	using MeshResourceRef = ResourceRef<std::vector<Renderer::Mesh>>;
}

namespace Nork::Components
{
	struct Drawable
	{
		MeshResourceRef resource;
	};
}