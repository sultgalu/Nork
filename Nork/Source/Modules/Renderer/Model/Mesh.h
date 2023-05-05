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
};

struct Model // sharable across entities (not linked with entity specific data eg. Transform)
{
	std::vector<MeshNode> nodes;
};
}