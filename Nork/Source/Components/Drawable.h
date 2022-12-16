#pragma once

namespace Nork::Components
{
	struct Mesh
	{
		// Renderer::Mesh mesh; // assigne only with entt::patch
		// Renderer::Material material; // assigne only with entt::patch
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
		// std::vector<Renderer::ModelMatrixRef> transforms; // global transforms of each mesh (if mesh.localTransform is empty, it is the shared transform)
		// Renderer::ModelMatrixRef sharedTransform; // transform of the model itself and meshes that do not have a local transform
	private:
		std::shared_ptr<Model> model;
	};
}