#include "../Drawable.h"
#include "Core/RenderingSystem.h"

namespace Nork::Components {
	void Drawable::SetModel(std::shared_ptr<Model> model_)
	{
		model = model_;
		transforms.clear();
		transforms.reserve(model->nodes.size());
		for (auto& node : model->nodes)
			transforms.push_back(node.localTransform.has_value() ? RenderingSystem::Instance().NewModelMatrix() : sharedTransform);
	}
}
