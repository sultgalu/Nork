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
	std::shared_ptr<Mesh> mesh;
	std::optional<glm::mat4> localTransform;
	std::vector<int> children;
};

struct Model // sharable across entities (not linked with entity specific data eg. Transform)
{
	std::vector<MeshNode> nodes;
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
	std::vector<std::shared_ptr<BufferElement<glm::mat4>>> childTransforms; // global transforms of each mesh (if mesh.localTransform is empty, it uses the shared transform)
};
}