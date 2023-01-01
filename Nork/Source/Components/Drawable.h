#pragma once

#include "Modules/Renderer/Model/Material.h"
#include "Modules/Renderer/Model/Mesh.h"

namespace Nork::Components
{
	struct Mesh
	{
		std::shared_ptr<Renderer::Mesh> mesh; // assign only with entt::patch
		std::shared_ptr<Renderer::Material> material; // assign only with entt::patch
		std::optional<glm::mat4> localTransform = std::nullopt;
	};

	struct Model // sharable across entities (not linked with entity specific data eg. Transform)
	{
		std::vector<Mesh> meshes;
	};

	struct Drawable
	{
		void SetModel(std::shared_ptr<Model>);
		inline std::shared_ptr<Model> GetModel() const { return model; }
		std::vector<std::shared_ptr<Renderer::BufferElement<glm::mat4>>> transforms; // global transforms of each mesh (if mesh.localTransform is empty, it is the shared transform)
		std::shared_ptr<Renderer::BufferElement<glm::mat4>> sharedTransform; // transform of the model itself and meshes that do not have a local transform
	private:
		std::shared_ptr<Model> model;
	};
}