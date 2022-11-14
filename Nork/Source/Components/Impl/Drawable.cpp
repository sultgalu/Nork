#include "../Drawable.h"
#include "Core/RenderingSystem.h"

namespace Nork::Components {
	void Drawable::SetModel(std::shared_ptr<Model> model_)
	{
		model = model_;
		transforms.clear();
		transforms.reserve(model->meshes.size());
		for (auto& mesh : model->meshes)
			transforms.push_back(mesh.localTransform.has_value() ? RenderingSystem::Instance().world.AddTransform() : sharedTransform);
	}
}
