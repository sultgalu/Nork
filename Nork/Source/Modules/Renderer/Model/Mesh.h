#pragma once

#include "../Data/Vertex.h"
#include "../DeviceData.h"
#include "Material.h"

namespace Nork::Renderer {

class MeshData
{
public:
	MeshData() = default;
	MeshData(std::shared_ptr<BufferView<Data::Vertex>> vertices, std::shared_ptr<BufferView<uint32_t>> indices)
		: vertices(vertices), indices(indices)
	{}
public:
	std::shared_ptr<BufferView<Data::Vertex>> vertices;
	std::shared_ptr<BufferView<uint32_t>> indices;
};

struct Primitive // can promote a submesh to a mesh, after that it would have it's own local transform
{
	std::shared_ptr<MeshData> meshData;
	std::shared_ptr<Material> material;
	ShadingMode shadingMode = ShadingMode::Default;
};
struct Mesh {
	std::vector<Primitive> primitives;
};
struct MeshNode { // a mesh with local transform information (local to model)
	std::shared_ptr<Mesh> mesh = nullptr; // can be empty
	std::optional<glm::mat4> localTransform;
	std::vector<int> children;
};

struct Animation {
	std::string name;
	enum class Path {
		Translation, Rotation, Scale, Weights
	};
	enum class Interpolation {
		Linear, Step, CubicSpline
	};
	struct Sampler {
		Interpolation interpolation;
		std::vector<float> timestamps;
		std::vector<float> keyFrameData;
		template<class T> std::span<T> KeyFrame() { return std::span((T*)keyFrameData.data(), keyFrameData.size() / (sizeof(T) / sizeof(float))); }
	};
	struct Channel {
		int samplerIdx;
		int node;
		Path path;
	};
	std::vector<Sampler> samplers;
	std::vector<Channel> channels;
};

struct Model // sharable across entities (not linked with entity specific data eg. Transform)
{
	std::vector<MeshNode> nodes;
	std::vector<Animation> animations;
};

struct Object // sharable across entities (not linked with entity specific data eg. Transform)
{
	void SetModel(std::shared_ptr<Model> model_);
	void SetBufferElementRecursive(int nodeIdx, std::shared_ptr<BufferElement<glm::mat4>>& parentMM);
	void SetTransform(const glm::mat4& modelMatrix);
	void SetTransformRecursive(int nodeIdx, const glm::mat4& parentMM);
	std::shared_ptr<Model> GetModel() { return model; }
// private:
	std::shared_ptr<Model> model;
	std::shared_ptr<BufferElement<glm::mat4>> transform; // transform of the model itself and meshes that do not have a local transform
	glm::mat4 modelMatrix;
	std::vector<std::shared_ptr<BufferElement<glm::mat4>>> childTransforms; // global transforms of each mesh (if mesh.localTransform is empty, it uses the shared transform)
	
	std::vector<std::optional<glm::mat4>> defaultTransforms; // local transform of each node saved when animating
	bool isAnimating = false;
	float animationTime = 0.0;
	struct Transform {
		glm::vec3 translation, scale;
		glm::quat rotation;
		glm::mat4 Matrix();
	};
	std::vector<Transform> animatedTransforms;
	float animationMaxTime;

	void StartAnimation();
	void UpdateAnimation(float delta);
	void StopAnimation();
};
}