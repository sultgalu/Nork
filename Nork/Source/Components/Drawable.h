#pragma once

#include "Modules/Renderer/Model/Material.h"
#include "Modules/Renderer/Model/Mesh.h"

namespace Nork::Components
{

struct Model // sharable across entities (not linked with entity specific data eg. Transform)
{
	struct SubMesh // can promote a submesh to a mesh, after that it would have it's own local transform
	{
		std::shared_ptr<Renderer::Mesh> mesh; // assign only with entt::patch
		std::shared_ptr<Renderer::Material> material; // assign only with entt::patch
	};
	struct Mesh {
		std::vector<SubMesh> subMeshes;
	};
	struct MeshNode { // a mesh with local transform information (local to model)
		std::shared_ptr<Mesh> mesh;
		std::optional<glm::mat4> localTransform;
	};

	std::vector<MeshNode> nodes;
};
struct Drawable
{
	void SetModel(std::shared_ptr<Model>);
	inline std::shared_ptr<Model> GetModel() const { return model; } // prevents you from setting the model property directly
	std::vector<std::shared_ptr<Renderer::BufferElement<glm::mat4>>> transforms; // global transforms of each mesh (if mesh.localTransform is empty, it is the shared transform)
	std::shared_ptr<Renderer::BufferElement<glm::mat4>> sharedTransform; // transform of the model itself and meshes that do not have a local transform
private:
	std::shared_ptr<Model> model;
};
}