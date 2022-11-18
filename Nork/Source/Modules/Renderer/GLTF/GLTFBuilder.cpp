#include "GLTFBuilder.h"

namespace Nork::Renderer {

	GLTFBuilder& GLTFBuilder::AddScene(bool setDefault)
	{
		if (setDefault)
			gltf.scene = gltf.scenes.size();
		gltf.scenes.push_back(GLTF::Scene());
		return *this;
	}

	GLTFBuilder& GLTFBuilder::AddMesh(const Mesh& mesh, const std::string& name, int matIdx, const std::filesystem::path& buffersPath)
    { // adds a node too
		gltf.scenes.back().nodes.push_back(gltf.nodes.size());
		gltf.nodes.push_back(GLTF::Node());
		gltf.nodes.back().mesh = gltf.meshes.size();

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

		GLTF::Mesh glMesh;

		GLTF::Buffer verticesBuffer; // 0
		GLTF::Buffer indicesBuffer; // 1
		GLTF::BufferView posView; // 0
		GLTF::BufferView normView; // 1
		GLTF::BufferView texCoordView; // 2
		GLTF::BufferView tangView; // 3
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

		glMesh.name = name;
		glMesh.primitives = { prim };

		verticesBuffer.uri = glMesh.name;
		indicesBuffer.uri = glMesh.name;
		verticesBuffer.byteLength = mesh.Vertices().SizeBytes();
		indicesBuffer.byteLength = mesh.Indices().SizeBytes();
		if (!buffersPath.empty())
		{
			verticesBuffer.uri += ".vertices";
			indicesBuffer.uri += ".indices";
			FileUtils::WriteBinary(mesh.Vertices().Data(), mesh.Vertices().SizeBytes(), std::filesystem::path(buffersPath).append(verticesBuffer.uri).string());
			FileUtils::WriteBinary(mesh.Indices().Data(), mesh.Indices().SizeBytes(), std::filesystem::path(buffersPath).append(indicesBuffer.uri).string());
		}
		using Vertex = Data::Vertex;
		auto setBufferView = [&](GLTF::BufferView& bufferView, int size, int offset)
		{
			bufferView.buffer = bufIdx(0);
			bufferView.byteLength = mesh.Vertices().SizeBytes() - offset;
			bufferView.byteOffset = offset;
			bufferView.byteStride = sizeof(Vertex);
			bufferView.target = GL_ARRAY_BUFFER;
		};
		setBufferView(posView, sizeof(Vertex::position), offsetof(Vertex, position));
		setBufferView(normView, sizeof(Vertex::normal), offsetof(Vertex, normal));
		setBufferView(texCoordView, sizeof(Vertex::texCoords), offsetof(Vertex, texCoords));
		setBufferView(tangView, sizeof(Vertex::tangent), offsetof(Vertex, tangent));
		indView.buffer = bufIdx(1);
		indView.byteLength = indicesBuffer.byteLength;
		indView.byteOffset = 0;
		indView.byteStride = 0;
		indView.target = GL_ELEMENT_ARRAY_BUFFER;

		auto setAccessor = [&](GLTF::Accessor& accessor, int bufViewIdx, const std::string& type, int comp = GL_FLOAT)
		{
			accessor.bufferView = bufViewIdx;
			accessor.byteOffset = 0;
			accessor.type = type;
			accessor.componentType = comp;
			accessor.count = mesh.Vertices().Count();
		};
		setAccessor(posAcc, bufViewIdx(0), GLTF::Accessor::VEC3);
		// TODO use again when min/max comp count fixed
		// posAcc.min = posMin;
		// posAcc.max = posMax;
		setAccessor(normAcc, bufViewIdx(1), GLTF::Accessor::VEC3);
		setAccessor(texCoordAcc, bufViewIdx(2), GLTF::Accessor::VEC2);
		setAccessor(tangAcc, bufViewIdx(3), GLTF::Accessor::VEC3);
		indAcc.bufferView = bufViewIdx(4);
		indAcc.byteOffset = 0;
		indAcc.type = GLTF::Accessor::SCALAR;
		indAcc.componentType = GL_UNSIGNED_INT;
		indAcc.count = mesh.Indices().Count();

		gltf.buffers.push_back(verticesBuffer);
		gltf.buffers.push_back(indicesBuffer);

		gltf.bufferViews.push_back(posView);
		gltf.bufferViews.push_back(normView);
		gltf.bufferViews.push_back(texCoordView);
		gltf.bufferViews.push_back(tangView);
		gltf.bufferViews.push_back(indView);

		gltf.accessors.push_back(indAcc);
		gltf.accessors.push_back(posAcc);
		gltf.accessors.push_back(normAcc);
		gltf.accessors.push_back(texCoordAcc);
		gltf.accessors.push_back(tangAcc);

		gltf.meshes.push_back(glMesh);

		return *this;
    }
    GLTFBuilder& GLTFBuilder::AddMaterial(const Material& material, std::vector<std::pair<TextureMap, std::string>> imageUris, const std::string& name)
    {
		GLTF::Material mat;
		mat.name = name;
		mat.pbrMetallicRoughness.baseColorFactor = glm::vec4(material->baseColorFactor, 1.0f);
		mat.pbrMetallicRoughness.roughnessFactor = material->roughnessFactor;
		// mat.pbrMetallicRoughness.extras = JsonObject().Property("specularExponent", material->specularExponent);

		for (auto [mapType, uri] : imageUris)
		{ // adds new image+texture even if uri already exists. Should search for uri and use that index if found.
			GLTF::TextureInfo texInfo;
			texInfo.index = gltf.images.size();

			GLTF::Image image;
			GLTF::Texture texture;
			image.uri = uri;
			texture.source = gltf.images.size();
			gltf.images.push_back(image);
			gltf.textures.push_back(texture);

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
			default:
				break;
			}
		}
		gltf.materials.push_back(mat);

		return *this;
    }
}
