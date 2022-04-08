#include "GLTFBuilder.h"

namespace Nork::Renderer {
	static GLTF::Buffer WriteIndexBuffer(std::shared_ptr<Renderer::Mesh> mesh, GLTF::Buffer& buffer, const std::string& path)
	{
		buffer.byteLength = mesh->GetIndexCount() * sizeof(uint32_t);
		FileUtils::WriteBinary(*mesh->GetIndexPtr().get(), buffer.byteLength, path);
		return buffer;
	}
	static GLTF::Buffer WriteVertexBuffer(std::shared_ptr<Renderer::Mesh> mesh, GLTF::Buffer& buffer, const std::string& path)
	{
		buffer.byteLength = mesh->GetVertexCount() * sizeof(Renderer::Data::Vertex);
		FileUtils::WriteBinary(*mesh->GetVertexPtr().get(), buffer.byteLength, path);
		return buffer;
	}

	GLTFBuilder& GLTFBuilder::AddScene(const std::vector<int>& nodes, bool defaultScene)
	{
		GLTF::Scene scene;
		scene.nodes = nodes;
		gltf.scenes.push_back(scene);
		if (defaultScene)
		{
			gltf.scene = gltf.scenes.size() - 1;
		}
		return *this;
	}

	GLTFBuilder& GLTFBuilder::AddNode(int meshIdx)
	{
		GLTF::Node node;
		node.mesh = meshIdx;
		gltf.nodes.push_back(node);
		return *this;
	}

	GLTFBuilder& GLTFBuilder::AddMesh(std::shared_ptr<Mesh> mesh, const std::string& name, int matIdx)
    {
		glm::vec3 posMin = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 posMax = glm::vec3(-std::numeric_limits<float>::max());
		for (size_t i = 0; i < mesh->GetVertexCount(); i++)
		{
			auto& vertex = (*mesh->GetVertexPtr())[i];
			if (vertex.position.x > posMax.x) posMax.x = vertex.position.x;
			if (vertex.position.y > posMax.y) posMax.y = vertex.position.y;
			if (vertex.position.z > posMax.z) posMax.z = vertex.position.z;

			if (vertex.position.x < posMin.x) posMin.x = vertex.position.x;
			if (vertex.position.y < posMin.y) posMin.y = vertex.position.y;
			if (vertex.position.z < posMin.z) posMin.z = vertex.position.z;
		}

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

		verticesBuffer.uri = glMesh.name + "_verts.bin";
		indicesBuffer.uri = glMesh.name + "_inds.bin";
		WriteIndexBuffer(mesh, indicesBuffer, std::filesystem::path(this->buffersPath).append(indicesBuffer.uri).string());
		WriteVertexBuffer(mesh, verticesBuffer, std::filesystem::path(this->buffersPath).append(verticesBuffer.uri).string());

		using Vertex = Data::Vertex;
		auto setBufferView = [&](GLTF::BufferView& bufferView, int size, int offset)
		{
			bufferView.buffer = bufIdx(0);
			bufferView.byteLength = mesh->GetVertexCount() * sizeof(Vertex) - offset;
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
			accessor.count = mesh->GetVertexCount();
		};
		setAccessor(posAcc, bufViewIdx(0), GLTF::Accessor::VEC3);
		posAcc.min = posMin;
		posAcc.max = posMax;
		setAccessor(normAcc, bufViewIdx(1), GLTF::Accessor::VEC3);
		setAccessor(texCoordAcc, bufViewIdx(2), GLTF::Accessor::VEC2);
		setAccessor(tangAcc, bufViewIdx(3), GLTF::Accessor::VEC3);
		indAcc.bufferView = bufViewIdx(4);
		indAcc.byteOffset = 0;
		indAcc.type = GLTF::Accessor::SCALAR;
		indAcc.componentType = GL_UNSIGNED_INT;
		indAcc.count = mesh->GetIndexCount();

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
    GLTFBuilder& GLTFBuilder::AddMaterial(std::shared_ptr<Material> material, const std::string& name, std::vector<std::pair<TextureMap, std::string>> imageUris)
    {
		GLTF::Material mat;
		mat.name = name;
		mat.pbrMetallicRoughness.baseColorFactor = glm::vec4(material->diffuse, 1.0f);
		mat.pbrMetallicRoughness.roughnessFactor = 1 - material->specular;
		mat.pbrMetallicRoughness.extras = JsonObject().Property("specularExponent", material->specularExponent);

		for (auto [mapType, uri] : imageUris)
		{
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
			case Diffuse:
				mat.pbrMetallicRoughness.baseColorTexture = texInfo;
				break;
			case Normal:
				mat.normalTexture = texInfo;
				break;
			case Roughness:
				mat.pbrMetallicRoughness.metallicRoughnessTexture = texInfo;
				break;
			case Reflection:
				MetaLogger().Error("Separate Relfection map not handled here.");
				break;
			default:
				break;
			}
		}
		gltf.materials.push_back(mat);

		return *this;
    }
}
