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
	this->modelMatrix = modelMatrix;
	*transform = modelMatrix;
	for (size_t i = 0; i < model->nodes.size(); i++) {
		SetTransformRecursive(0, modelMatrix); // TODO: should traverse it as a tree instead (start from root)
	}
}
void Object::StartAnimation() {
	animationTime = 0.0;
	isAnimating = true;
	animationMaxTime = 0;

	for (auto& animation : model->animations) { // DEBUG
		for (auto& channel : animation.channels) {
			if (!model->nodes[channel.node].localTransform.has_value()) {
				model->nodes[channel.node].localTransform = glm::identity<glm::mat4>();
				childTransforms[channel.node] = Resources::Instance().modelMatrices->New(); // add unique transform buffer element
			}
		}
		for (auto& sampler : animation.samplers) {
			if (animationMaxTime < sampler.timestamps.back()) {
				animationMaxTime = sampler.timestamps.back();
			}
		}
	}
	animatedTransforms.resize(model->nodes.size());
	defaultTransforms.clear();
	defaultTransforms.reserve(model->nodes.size());
	for (size_t i = 0; i < model->nodes.size(); i++) {
		defaultTransforms.push_back(model->nodes[i].localTransform);
		animatedTransforms[i].scale = glm::vec3(1);
		animatedTransforms[i].rotation = glm::identity<glm::quat>();
		animatedTransforms[i].translation = glm::vec3(0);
	}
}
void Object::UpdateAnimation(float delta) {
	if (animationMaxTime <= animationTime) {
		StopAnimation();
		return;
	}
	delta = std::min(delta, animationMaxTime - animationTime); // clamp to end
	animationTime += delta;

	std::unordered_set<int> animatedNodes;

	for (auto& animation : model->animations) {
		for (auto& channel : animation.channels) {
			auto& tr = animatedTransforms[channel.node];
			auto& sampler = animation.samplers[channel.samplerIdx];
			float ratio = 1; // clamp end
			int begin = sampler.timestamps.size() - 1, end = begin; // clamp end
			int tsIdx = -1;
			for (size_t i = 0; i < sampler.timestamps.size(); i++) {
				if (animationTime < sampler.timestamps[i]) {
					if (i == 0) {
						begin = i;
						end = i + 1; // this causes crash if ts.size() == 1
						ratio = 0; // clamp begin
					}
					else {
						begin = i - 1;
						end = i;
						tsIdx = i;
						ratio = (animationTime - sampler.timestamps[begin]) / (sampler.timestamps[end] - sampler.timestamps[begin]);
					}
					break;
				}
			}
			if (true || tsIdx != -1) {
				animatedNodes.insert(channel.node);
				if (channel.path == Animation::Path::Translation) {
					tr.translation = sampler.KeyFrame<glm::vec3>()[begin] * (1 - ratio) + sampler.KeyFrame<glm::vec3>()[end] * ratio;
				}
				else if (channel.path == Animation::Path::Scale) {
					tr.scale = sampler.KeyFrame<glm::vec3>()[begin] * (1 - ratio) + sampler.KeyFrame<glm::vec3>()[end] * ratio;
				}
				else if (channel.path == Animation::Path::Rotation) {
					tr.rotation = sampler.KeyFrame<glm::quat>()[begin] * (1 - ratio) + sampler.KeyFrame<glm::quat>()[end] * ratio;
					tr.rotation = glm::normalize(tr.rotation);
				}
				else {
					std::unreachable();
				}
			}
		}
	}
	for (auto nodeIdx : animatedNodes) { // update on gpu
		model->nodes[nodeIdx].localTransform = animatedTransforms[nodeIdx].Matrix();
	}
	SetTransform(modelMatrix);
}
void Object::StopAnimation() {
	isAnimating = false;
	//SetModel(model);
	for (size_t i = 0; i < model->nodes.size(); i++) {
		model->nodes[i].localTransform = defaultTransforms[i];
	}
	SetTransform(modelMatrix);
	defaultTransforms.clear();
}
glm::mat4 Object::Transform::Matrix()
{
	return glm::translate(glm::identity<glm::mat4>(), translation) * glm::mat4_cast(rotation) * glm::scale(glm::identity<glm::mat4>(), scale);
}
}