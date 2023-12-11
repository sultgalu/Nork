#pragma once

#include "../Data/Vertex.h"
#include "../DeviceData.h"
#include "Material.h"

namespace Nork::Renderer {

struct MeshData {
	enum VertexDataType {
		Default, Skinned
	};

	virtual Buffer& VertexBuffer() = 0;
	virtual Buffer& IndexBuffer() = 0;
	virtual uint32_t VertexBufferOffset() const = 0;
	virtual uint32_t IndexBufferOffset() const = 0;
	virtual uint32_t VertexCount() const = 0;
	virtual uint32_t IndexCount() const = 0;
	virtual uint32_t VertexSizeBytes() const = 0;
	virtual uint32_t IndexSizeBytes() const = 0;
	virtual VertexDataType VertexType() const = 0;
};

template<class VT>
struct MeshDataImpl: MeshData { // TODO: template index buffer by uint32_t/uint16_t/uint8_t
	MeshDataImpl() = default;
	MeshDataImpl(std::shared_ptr<BufferView<VT>> vertices, std::shared_ptr<BufferView<uint32_t>> indices)
		: vertices(vertices), indices(indices)
	{
	}
	Buffer& VertexBuffer() override { return *vertices->buffer; }
	Buffer& IndexBuffer() override { return *indices->buffer; }
	uint32_t VertexBufferOffset() const override { return vertices->offset; }
	uint32_t IndexBufferOffset() const override { return indices->offset; }
	uint32_t VertexCount() const override { return vertices->count; }
	uint32_t IndexCount() const override { return indices->count; }
	uint32_t VertexSizeBytes() const override { return vertices->SizeBytes(); }
	uint32_t IndexSizeBytes() const override { return indices->SizeBytes(); }
	VertexDataType VertexType() const override { return std::is_same_v<VT, Data::VertexSkinned> ? Skinned : Default; }
	std::shared_ptr<BufferView<VT>> vertices;
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
	bool bone = false; // what to do when bone's children has a mesh? gltf spec says some stuff
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

struct Skin {
	std::string name;
	int skeletonNode;
	bool HasSkeleton() { return skeletonNode != -1; }
	std::vector<uint16_t> joints;
	std::vector<std::shared_ptr<BufferElement<glm::mat4>>> inverseBindMatrices;
};

struct Model // sharable across entities (not linked with entity specific data eg. Transform)
{
	std::vector<MeshNode> nodes;
	std::vector<Animation> animations;
	std::vector<Skin> skins;
};

struct Object
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