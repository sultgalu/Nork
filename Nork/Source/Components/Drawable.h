#pragma once

#include "Modules/Renderer/Model/Material.h"
#include "Modules/Renderer/Model/Mesh.h"

namespace Nork::Components
{

struct Drawable
{
	void SetModel(std::shared_ptr<Renderer::Model>);
	inline std::shared_ptr<Renderer::Model> GetModel() const { return model; } // prevents you from setting the model property directly
	std::vector<std::shared_ptr<Renderer::BufferElement<glm::mat4>>> transforms; // global transforms of each mesh (if mesh.localTransform is empty, it uses the shared transform)
	std::shared_ptr<Renderer::BufferElement<glm::mat4>> sharedTransform; // transform of the model itself and meshes that do not have a local transform
private:
	std::shared_ptr<Renderer::Model> model;
};
}