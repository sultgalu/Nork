#include "Mesh.h"
#include "../Resources.h"

namespace Nork::Renderer {
static std::unordered_set<int> processed;

void Object::SetBufferElementRecursive(int nodeIdx, std::shared_ptr<BufferElement<glm::mat4>>& parentMM)
{
	auto& node = model->nodes[nodeIdx];
	if (processed.contains(nodeIdx)) {
		return;
	}
	processed.insert(nodeIdx);

	auto mm = node.localTransform.has_value() ? Resources::Instance().modelMatrices->New() : parentMM;
	childTransforms[nodeIdx] = mm;
	for (auto& i : node.children) {
		SetBufferElementRecursive(i, mm);
	}
}
void Object::SetModel(std::shared_ptr<Model> model_)
{
	model = model_;
	childTransforms.clear();
	childTransforms.resize(model->nodes.size());
	processed.clear();
	for (size_t i = 0; i < model->nodes.size(); i++) {
		SetBufferElementRecursive(i, transform);
	}
}
void Object::SetTransformRecursive(int nodeIdx, const glm::mat4& parentMM) {
	auto& node = model->nodes[nodeIdx];
	if (processed.contains(nodeIdx)) {
		return;
	}
	processed.insert(nodeIdx);

	auto mm = node.localTransform.has_value() ? parentMM * *node.localTransform : parentMM;
	if (node.localTransform.has_value()) {
		*childTransforms[nodeIdx] = mm;
	}
	for (auto& i : node.children) {
		SetTransformRecursive(i, mm);
	}
}
void Object::SetTransform(const glm::mat4& modelMatrix) {
	processed.clear();
	*transform = modelMatrix;
	int nodeIdx = 0;
	for (size_t i = 0; i < model->nodes.size(); i++) {
		SetTransformRecursive(i, modelMatrix);
	}
}
}