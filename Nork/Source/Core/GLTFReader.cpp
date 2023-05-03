#include "GLTFReader.h"
#include "Core/AssetLoader.h"

namespace Nork {
GLTFReader::GLTFReader(const fs::path& path)
{
	srcFolder = path.parent_path();
	dstFolder = path.stem();
	gltf = Renderer::GLTF::GLTF::FromJson(JsonObject::ParseFormatted(FileUtils::ReadAsString(path.string())));
}
std::shared_ptr<Components::Model> GLTFReader::Read()
{
	model = std::make_shared<Components::Model>();

	for (auto& buf : gltf.buffers)
		buffers.push_back(FileUtils::ReadBinary<char>(AbsolutePath(buf.uri).string()));
	for (auto& img : gltf.images)
		images.push_back(AssetLoader::Instance().LoadTexture(img.uri)); // this is copied over by AssetLoader to the right folder
	for (auto& mat : gltf.materials)
		materials.push_back(CreateMaterial(mat));
	for (size_t i = 0; i < gltf.meshes.size(); i++)
	{
		meshes.push_back({});
		for (size_t j = 0; j < gltf.meshes[i].primitives.size(); j++)
		{
			auto mesh = CreateRendererMesh(j, i);
			meshes.back().push_back(mesh);
		}
	}
	for (auto nodeIdx : gltf.scenes.back().nodes)
		AddNodeRecursive(nodeIdx);

	return model;
}
void GLTFReader::AddNodeRecursive(int nodeIdx, const glm::mat4& parentTransform)
{
	const Renderer::GLTF::Node& glNode = gltf.nodes[nodeIdx];
	glm::mat4 transform = parentTransform;
	if (glNode.HasTransform()) // "The global transformation matrix of a node is the product of the global transformation matrix of its parent node and its own local transformation matrix"
		transform = parentTransform * glNode.Transform();

	for (auto i : glNode.children)
		AddNodeRecursive(i, transform);

	if (glNode.mesh != -1)
	{
		auto& glMesh = gltf.meshes[glNode.mesh];
		model->nodes.push_back(Components::Model::MeshNode{ .mesh = std::make_shared<Components::Model::Mesh>() });
		for (size_t i = 0; i < glMesh.primitives.size(); i++)
		{
			auto& mesh = meshes[glNode.mesh][i];
			if (mesh)
			{
				auto matIdx = glMesh.primitives[i].material;
				model->nodes.back().mesh->subMeshes.push_back(Components::Model::SubMesh{
					.mesh = mesh,
					.material = matIdx == -1 ? defaultMaterial : materials[matIdx]
					});
			}
		}
		if (transform != glm::identity<glm::mat4>()) {
			model->nodes.back().localTransform = transform;
		}
	}
}
std::shared_ptr<Renderer::Material> GLTFReader::CreateMaterial(const Renderer::GLTF::Material& mat)
{
	auto material = RenderingSystem::Instance().NewMaterial();
	auto data = material->Data();
	data->baseColorFactor = mat.pbrMetallicRoughness.baseColorFactor;
	data->roughnessFactor = mat.pbrMetallicRoughness.roughnessFactor;
	data->metallicFactor = mat.pbrMetallicRoughness.metallicFactor;
	data->emissiveFactor = mat.emissiveFactor;
	if (mat.alphaMode == mat.MASK)
		data->alphaCutoff = mat.alphaCutoff;
	else if (mat.alphaMode == mat.BLEND)
		material->shadingMode = Renderer::ShadingMode::Blend;
	// material->specularExponent = mat.pbrMetallicRoughness.extras.Get<float>("specularExponent"); has official extension
	if (mat.pbrMetallicRoughness.baseColorTexture.Validate())
		material->SetTextureMap(images[gltf.textures[mat.pbrMetallicRoughness.baseColorTexture.index].source], Renderer::TextureMap::BaseColor);
	if (mat.pbrMetallicRoughness.metallicRoughnessTexture.Validate())
		material->SetTextureMap(images[gltf.textures[mat.pbrMetallicRoughness.metallicRoughnessTexture.index].source], Renderer::TextureMap::MetallicRoughness);
	if (mat.normalTexture.Validate())
		material->SetTextureMap(images[gltf.textures[mat.normalTexture.index].source], Renderer::TextureMap::Normal);
	if (mat.occlusionTexture.Validate())
		material->SetTextureMap(images[gltf.textures[mat.occlusionTexture.index].source], Renderer::TextureMap::Occlusion);
	if (mat.emissiveTexture.Validate())
		material->SetTextureMap(images[gltf.textures[mat.emissiveTexture.index].source], Renderer::TextureMap::Emissive);
	return material;
}
std::shared_ptr<Renderer::Mesh> GLTFReader::CreateRendererMesh(int idx, int meshIdx)
{
	using namespace Renderer;

	auto& prim = gltf.meshes[meshIdx].primitives[idx];
	if (prim.mode != GL_TRIANGLES)
	{
		Logger::Warning("skipping primitive ", idx, " of mesh ", meshIdx, ", its mode is not GL_TRIANGLES, but ", prim.mode);
		return nullptr;
	}
	int posAccIdx = prim.Accessor(GLTF::Attribute::position);
	int normAccIdx = prim.Accessor(GLTF::Attribute::normal);
	int texAccIdx = prim.Accessor(GLTF::Attribute::texcoord0);
	if (posAccIdx == -1 || normAccIdx == -1 || texAccIdx == -1)
	{
		Logger::Warning("skipping primitive ", idx, " of mesh ", meshIdx, ", incorrect vertex accessors (missing accessor)");
		return nullptr;
	}
	auto posAcc = gltf.accessors[posAccIdx];
	auto normAcc = gltf.accessors[normAccIdx];
	auto texAcc = gltf.accessors[texAccIdx];
	auto idxAcc = gltf.accessors[prim.indices];
	if (idxAcc.type != GLTF::Accessor::SCALAR || posAcc.type != GLTF::Accessor::VEC3 || normAcc.type != GLTF::Accessor::VEC3 || texAcc.type != GLTF::Accessor::VEC2)
	{
		Logger::Warning("skipping primitive ", idx, " of mesh ", meshIdx, ", incorrect vertex accessors (bad type)");
		return nullptr;
	}
	if (posAcc.componentType != GL_FLOAT || normAcc.componentType != GL_FLOAT || texAcc.componentType != GL_FLOAT)
	{
		Logger::Warning("skipping primitive ", idx, " of mesh ", meshIdx, ", incorrect vertex accessors (bad componentType)");
		return nullptr;
	}
	if (posAcc.count != normAcc.count || normAcc.count != texAcc.count)
	{
		Logger::Warning("skipping primitive ", idx, " of mesh ", meshIdx, ", incorrect vertex accessors (counts not equal)");
		return nullptr;
	}
	auto posStride = glm::max<int>(gltf.bufferViews[posAcc.bufferView].byteStride, sizeof(glm::vec3)) / sizeof(float);
	auto normStride = glm::max<int>(gltf.bufferViews[normAcc.bufferView].byteStride, sizeof(glm::vec3)) / sizeof(float);
	auto texStride = glm::max<int>(gltf.bufferViews[texAcc.bufferView].byteStride, sizeof(glm::vec2)) / sizeof(float);
	std::span<float> posData = BufferView<glm::vec3>(posAcc);
	std::span<float> normData = BufferView<glm::vec3>(normAcc);
	std::span<float> texData = BufferView<glm::vec2>(texAcc);

	std::vector<Data::Vertex> vertices;
	vertices.reserve(posAcc.count);
	for (size_t i = 0; i < posAcc.count; i++)
	{
		vertices.push_back(Data::Vertex{
			.position = *((glm::vec3*)&posData[i * posStride]),
			.normal = *((glm::vec3*)&normData[i * normStride]),
			.texCoords = *((glm::vec2*)&texData[i * texStride]),
			});
	}
	std::vector<uint32_t> indices;
	if (prim.indices == -1)
	{
		Logger::Warning("Generating Index buffer as an arithmetic series, because primitive ", idx, " of mesh ", meshIdx, ", does not provide one");
		indices.resize(vertices.size());
		uint32_t n = 0;
		std::ranges::generate(indices, [&n]() mutable { return n++; });
	}
	else if (idxAcc.componentType == GL_UNSIGNED_INT)
		indices = GetIndices<uint32_t>(prim);
	else if (idxAcc.componentType == GL_UNSIGNED_SHORT)
		indices = GetIndices<uint16_t>(prim);
	else if (idxAcc.componentType == GL_UNSIGNED_BYTE)
		indices = GetIndices<uint8_t>(prim);
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		SetTangent(vertices[indices[i]], vertices[indices[i + 1]], vertices[indices[i + 2]]);
	}
	return RenderingSystem::Instance().NewMesh(vertices, indices);
}
template<class T> std::vector<uint32_t> GLTFReader::GetIndices(const Renderer::GLTF::Primitive& prim)
{
	auto idxAcc = gltf.accessors[prim.indices];
	auto idxStride = glm::max<int>(gltf.bufferViews[idxAcc.bufferView].byteStride / sizeof(T), 1);;
	std::span<T> idxData = BufferView<T, T>(idxAcc);
	std::vector<uint32_t> indices(idxAcc.count);
	for (size_t i = 0; i < idxAcc.count; i++)
	{
		indices[i] = idxData[i * idxStride];
	}
	return indices;
}
void GLTFReader::SetTangent(Renderer::Data::Vertex& v1, Renderer::Data::Vertex& v2, Renderer::Data::Vertex& v3)
{
	glm::vec3 edge1 = v2.position - v1.position;
	glm::vec3 edge2 = v3.position - v1.position;
	glm::vec2 deltaUV1 = v2.texCoords - v1.texCoords;
	glm::vec2 deltaUV2 = v3.texCoords - v1.texCoords;

	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	glm::vec3 tangent;
	tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	v1.tangent = tangent;
	v2.tangent = tangent;
	v3.tangent = tangent;
}
template<class S, class T> std::span<T> GLTFReader::BufferView(const Renderer::GLTF::Accessor& accessor)
{
	auto bufferView = gltf.bufferViews[accessor.bufferView];
	auto start = buffers[bufferView.buffer].data() + bufferView.byteOffset + accessor.byteOffset;
	auto size = accessor.count * glm::max<int>(bufferView.byteStride, sizeof(S)); // if bytestride is not 0, it should not be less that sizeof(T)
	return std::span<T>((T*)start, size / sizeof(T));
}
}