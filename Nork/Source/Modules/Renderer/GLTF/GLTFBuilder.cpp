#include "GLTFBuilder.h"

namespace Nork::Renderer {

GLTFBuilder& GLTFBuilder::AddScene(bool setDefault)
{
	if (setDefault)
		gltf.scene = gltf.scenes.size();
	gltf.scenes.push_back(GLTF::Scene());

	return *this;
}
GLTFBuilder& GLTFBuilder::AddNode(int meshIdx)
{
	gltf.scenes.back().nodes.push_back(gltf.nodes.size());
	gltf.nodes.push_back(GLTF::Node());
	gltf.nodes.back().mesh = meshIdx;
	return *this;
}
GLTFBuilder& GLTFBuilder::AddTransform(const glm::mat4& tr) {
	/*glm::vec3 scale, translation, skew; glm::vec4 persp;
	glm::quat quat;
	glm::decompose(tr, scale, quat, translation, skew, persp);
	quat = glm::conjugate(quat);
	if (quat != glm::identity<glm::quat>()) {
		gltf.nodes.back().rotation = quat;
	}
	if (scale != glm::vec3(1)) {
		gltf.nodes.back().scale = scale;
	}
	if (translation != glm::vec3(0)) {
		gltf.nodes.back().translation = translation;
	}*/
	gltf.nodes.back().matrix = tr; // the other way
	return *this;
}

GLTFBuilder& GLTFBuilder::AddMesh(const std::string& name)
{
	GLTF::Mesh gltfMesh;
	if (!name.empty()) {
		gltfMesh.name = name;
	}
	gltf.meshes.push_back(gltfMesh);
	return *this;
}
GLTFBuilder& GLTFBuilder::AddPrimitive(const Mesh& mesh, const std::filesystem::path& buffersPath, int matIdx)
{ // adds a node too
	if (buffersPath.empty()) {
		std::unreachable();
	}

	// TODO use again when min/max comp count fixed
	// glm::vec3 posMin = glm::vec3(std::numeric_limits<float>::max());
	// glm::vec3 posMax = glm::vec3(-std::numeric_limits<float>::max());
	// for (size_t i = 0; i < mesh.Vertices().Count(); i++)
	// {
	// 	auto& vertex = mesh.Vertices()[i];
	// 	if (vertex.position.x > posMax.x) posMax.x = vertex.position.x;
	// 	if (vertex.position.y > posMax.y) posMax.y = vertex.position.y;
	// 	if (vertex.position.z > posMax.z) posMax.z = vertex.position.z;
	// 
	// 	if (vertex.position.x < posMin.x) posMin.x = vertex.position.x;
	// 	if (vertex.position.y < posMin.y) posMin.y = vertex.position.y;
	// 	if (vertex.position.z < posMin.z) posMin.z = vertex.position.z;
	// }

	auto bufIdx = [&](int idx) { return gltf.buffers.size() + idx; };
	auto bufViewIdx = [&](int idx) { return gltf.bufferViews.size() + idx; };
	auto accIdx = [&](int idx) { return gltf.accessors.size() + idx; };

	GLTF::Buffer verticesBuffer; // 0
	GLTF::Buffer indicesBuffer; // 1
	GLTF::BufferView vertView; // 0
	GLTF::BufferView indView; // 4

	GLTF::Accessor indAcc; // 0
	GLTF::Accessor posAcc; // 1
	GLTF::Accessor normAcc; // 2
	GLTF::Accessor texCoordAcc; // 3
	GLTF::Accessor tangAcc; // 4

	GLTF::Primitive prim;
	prim.indices = accIdx(0);
	prim.mode = GL_TRIANGLES;
	prim.attributes.push_back(GLTF::Attribute::POSITION(accIdx(1)));
	prim.attributes.push_back(GLTF::Attribute::NORMAL(accIdx(2)));
	prim.attributes.push_back(GLTF::Attribute::TEXCOORD_0(accIdx(3)));
	//prim.attributes.push_back(GLTF::Attribute::TANGENT(accIdx(4))); -> needs to be VEC4, w = 1/-1 indicating handedness
	if (matIdx >= 0)
		prim.material = matIdx;

	size_t meshIdx = gltf.meshes.size() - 1;
	size_t primIdx = gltf.meshes.back().primitives.size();
	verticesBuffer.uri = "vertices_" + std::to_string(meshIdx) + "_" + std::to_string(primIdx) + ".bin";
	indicesBuffer.uri = "indices_" + std::to_string(meshIdx) + "_" + std::to_string(primIdx) + ".bin";
	gltf.meshes.back().primitives.push_back(prim);

	verticesBuffer.byteLength = mesh.vertices->SizeBytes();
	indicesBuffer.byteLength = mesh.indices->SizeBytes();
	mesh.vertices->ReadAll([=](std::span<Data::Vertex> vertices) {
		FileUtils::WriteBinary(vertices.data(), vertices.size_bytes(), std::filesystem::path(buffersPath).append(verticesBuffer.uri).string());
	});
	mesh.indices->ReadAll([=](std::span<uint32_t> indices) {
		FileUtils::WriteBinary(indices.data(), indices.size_bytes(), std::filesystem::path(buffersPath).append(indicesBuffer.uri).string());
	});

	vertView.buffer = bufIdx(0);
	vertView.byteLength = mesh.vertices->SizeBytes();
	vertView.byteOffset = 0;
	vertView.byteStride = sizeof(Data::Vertex);
	vertView.target = GLTF::BufferView::ARRAY_BUFFER;

	indView.buffer = bufIdx(1);
	indView.byteLength = indicesBuffer.byteLength;
	indView.byteOffset = 0;
	indView.byteStride = 0;
	indView.target = GLTF::BufferView::ELEMENT_ARRAY_BUFFER;

	auto setAccessor = [&](GLTF::Accessor& accessor, int bufViewIdx, const std::string& type, int offset, int comp = GL_FLOAT)
	{
		accessor.bufferView = bufViewIdx;
		accessor.byteOffset = offset;
		accessor.type = type;
		accessor.componentType = comp;
		accessor.count = mesh.vertices->count;
	};
	using Vertex = Data::Vertex;
	setAccessor(posAcc, bufViewIdx(0), GLTF::Accessor::VEC3, offsetof(Vertex, position));
	// TODO use again when min/max comp count fixed
	// posAcc.min = posMin;
	// posAcc.max = posMax;
	setAccessor(normAcc, bufViewIdx(0), GLTF::Accessor::VEC3, offsetof(Vertex, normal));
	setAccessor(texCoordAcc, bufViewIdx(0), GLTF::Accessor::VEC2, offsetof(Vertex, texCoords));
	setAccessor(tangAcc, bufViewIdx(0), GLTF::Accessor::VEC3, offsetof(Vertex, tangent));
	indAcc.bufferView = bufViewIdx(1);
	indAcc.byteOffset = 0;
	indAcc.type = GLTF::Accessor::SCALAR;
	indAcc.componentType = GL_UNSIGNED_INT;
	indAcc.count = mesh.indices->count;

	gltf.buffers.push_back(verticesBuffer);
	gltf.buffers.push_back(indicesBuffer);

	gltf.bufferViews.push_back(vertView);
	gltf.bufferViews.push_back(indView);

	gltf.accessors.push_back(indAcc);
	gltf.accessors.push_back(posAcc);
	gltf.accessors.push_back(normAcc);
	gltf.accessors.push_back(texCoordAcc);
	gltf.accessors.push_back(tangAcc);

	return *this;
}
GLTFBuilder& GLTFBuilder::AddMaterial(const Material& material, std::vector<std::pair<TextureMap, std::string>> imageUris, const std::string& name)
{
	GLTF::Material mat;
	mat.name = name;
	mat.pbrMetallicRoughness.baseColorFactor = material->baseColorFactor;
	mat.pbrMetallicRoughness.roughnessFactor = material->roughnessFactor;
	mat.pbrMetallicRoughness.metallicFactor = material->metallicFactor;
	if (material->alphaCutoff != -1) {
		mat.alphaMode = mat.MASK;
		mat.alphaCutoff = material->alphaCutoff;
	}
	if (material.shadingMode == ShadingMode::Blend) {
		mat.alphaMode = mat.BLEND;
	}

	for (auto [mapType, uri] : imageUris)
	{ // adds new image+texture even if uri already exists. Should search for uri and use that index if found.
		if (!texturesByImageUri.contains(uri)) {
			GLTF::Image image;
			image.uri = uri;

			GLTF::Texture texture;
			texture.source = gltf.images.size();

			gltf.images.push_back(image);
			texturesByImageUri[uri] = gltf.textures.size();
			gltf.textures.push_back(texture);
		}

		GLTF::TextureInfo texInfo;
		texInfo.index = texturesByImageUri[uri];

		using enum TextureMap;
		switch (mapType)
		{
			case BaseColor:
				mat.pbrMetallicRoughness.baseColorTexture = texInfo;
				break;
			case Normal:
				mat.normalTexture = texInfo;
				break;
			case MetallicRoughness:
				mat.pbrMetallicRoughness.metallicRoughnessTexture = texInfo;
				break;
			case Occlusion:
				mat.occlusionTexture = texInfo;
				break;
			default:
				break;
		}
	}
	gltf.materials.push_back(mat);

	return *this;
}
}
