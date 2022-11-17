#pragma once
#include "ResourceManager.h"
#include "RenderingSystem.h"

namespace Nork{
	using namespace Renderer;
	class GLTFReader
	{
	public:
		GLTFReader(const fs::path& path)
		{
			srcFolder = path.parent_path();
			dstFolder = path.stem();
			gltf = GLTF::GLTF::FromJson(JsonObject::ParseFormatted(FileUtils::ReadAsString(path.string())));
		}
		std::shared_ptr<Components::Model> Read()
		{
			model = std::make_shared<Components::Model>();

			for (auto& buf : gltf.buffers)
				buffers.push_back(FileUtils::ReadBinary<char>(SrcPath(buf.uri).string()));
			for (auto& img : gltf.images)
				images.push_back(TextureResources::Instance().Get(SrcPath(img.uri)));
			for (auto& mat : gltf.materials)
				materials.push_back(CreateMaterial(mat));
			for (size_t i = 0; i < gltf.meshes.size(); i++)
			{
				meshes.push_back({});
				for (size_t j = 0; j < gltf.meshes[i].primitives.size(); j++)
				{
					auto mesh = CreateRendererMesh(j, i);
					MeshResources::Instance().Add(mesh, (dstFolder / std::to_string(i) / std::to_string(j)).replace_extension(".bin"));
					meshes.back().push_back(mesh);
				}
			}
			for (auto nodeIdx : gltf.scenes.back().nodes)
				AddMeshRecursive(nodeIdx);

			return model;
		}
		void AddMeshRecursive(int nodeIdx, const glm::mat4& parentTransform = glm::identity<glm::mat4>())
		{
			const GLTF::Node& node = gltf.nodes[nodeIdx];
			std::optional<glm::mat4> transform;
			if (node.HasTransform())
				transform = parentTransform * node.Transform();

			for (auto i : node.children)
				AddMeshRecursive(i, transform ? *transform : parentTransform);

			if (node.mesh != -1)
			{
				auto& meshGltf = gltf.meshes[node.mesh];
				for (size_t i = 0; i < meshGltf.primitives.size(); i++)
				{
					auto& mesh = meshes[node.mesh][i];
					if (!mesh.Empty())
					{
						auto matIdx = meshGltf.primitives[i].material;
						model->meshes.push_back(Components::Mesh{
							.mesh = mesh,
							.material = matIdx == -1 ? defaultMaterial : materials[matIdx],
							.localTransform = transform
							});
					}
				}
			}
		}
		Material CreateMaterial(const GLTF::Material& mat)
		{
			auto material = RenderingSystem::Instance().world.AddMaterial();
			material->baseColorFactor = mat.pbrMetallicRoughness.baseColorFactor;
			material->roughnessFactor = mat.pbrMetallicRoughness.roughnessFactor;
			material->metallicFactor = mat.pbrMetallicRoughness.metallicFactor;
			// material->specularExponent = mat.pbrMetallicRoughness.extras.Get<float>("specularExponent"); has official extension
			if (mat.pbrMetallicRoughness.baseColorTexture.Validate())
				material.SetTextureMap(images[gltf.textures[mat.pbrMetallicRoughness.baseColorTexture.index].source], Renderer::TextureMap::BaseColor);
			if (mat.pbrMetallicRoughness.metallicRoughnessTexture.Validate())
				material.SetTextureMap(images[gltf.textures[mat.pbrMetallicRoughness.metallicRoughnessTexture.index].source], Renderer::TextureMap::MetallicRoughness);
			if (mat.normalTexture.Validate())
				material.SetTextureMap(images[gltf.textures[mat.normalTexture.index].source], Renderer::TextureMap::Normal);
			return material;
		}
		Mesh CreateRendererMesh(int idx, int meshIdx)
		{
			auto& prim = gltf.meshes[meshIdx].primitives[idx];
			if (prim.mode != GL_TRIANGLES)
			{
				Logger::Warning("skipping primitive ", idx, " of mesh ", meshIdx, ", its mode is not GL_TRIANGLES, but ", prim.mode);
				return Mesh();
			}
			int posAccIdx = prim.Accessor(GLTF::Attribute::position);
			int normAccIdx = prim.Accessor(GLTF::Attribute::normal);
			int texAccIdx = prim.Accessor(GLTF::Attribute::texcoord0);
			if (posAccIdx == -1 || normAccIdx == -1 || texAccIdx == -1)
			{
				Logger::Warning("skipping primitive ", idx, " of mesh ", meshIdx, ", incorrect vertex accessors (missing accessor)");
				return Mesh();
			}
			auto posAcc = gltf.accessors[posAccIdx];
			auto normAcc = gltf.accessors[normAccIdx];
			auto texAcc = gltf.accessors[texAccIdx];
			auto idxAcc = gltf.accessors[prim.indices];
			if (idxAcc.type != GLTF::Accessor::SCALAR || posAcc.type != GLTF::Accessor::VEC3 || normAcc.type != GLTF::Accessor::VEC3 || texAcc.type != GLTF::Accessor::VEC2)
			{
				Logger::Warning("skipping primitive ", idx, " of mesh ", meshIdx, ", incorrect vertex accessors (bad type)");
				return Mesh();
			}
			if (posAcc.componentType != GL_FLOAT || normAcc.componentType != GL_FLOAT || texAcc.componentType != GL_FLOAT)
			{
				Logger::Warning("skipping primitive ", idx, " of mesh ", meshIdx, ", incorrect vertex accessors (bad componentType)");
				return Mesh();
			}
			if (posAcc.count != normAcc.count || normAcc.count != texAcc.count)
			{
				Logger::Warning("skipping primitive ", idx, " of mesh ", meshIdx, ", incorrect vertex accessors (counts not equal)");
				return Mesh();
			}
			auto posStride = glm::max<int>(gltf.bufferViews[posAcc.bufferView].byteStride, sizeof(glm::vec3)) / sizeof(float);
			auto normStride = glm::max<int>(gltf.bufferViews[normAcc.bufferView].byteStride, sizeof(glm::vec3)) / sizeof(float);
			auto texStride = glm::max<int>(gltf.bufferViews[texAcc.bufferView].byteStride, sizeof(glm::vec2)) / sizeof(float);
			std::span<float> posData = BufferView<glm::vec3>(posAcc);
			std::span<float> normData = BufferView<glm::vec3>(normAcc);
			std::span<float> texData = BufferView<glm::vec2>(texAcc);

			std::vector<Renderer::Data::Vertex> vertices;
			vertices.reserve(posAcc.count);
			for (size_t i = 0; i < posAcc.count; i++)
			{
				vertices.push_back(Renderer::Data::Vertex{
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
			return RenderingSystem::Instance().world.AddMesh(vertices, indices);
		}
		template<class T> std::vector<uint32_t> GetIndices(const GLTF::Primitive& prim)
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
		void SetTangent(Renderer::Data::Vertex& v1, Renderer::Data::Vertex& v2, Renderer::Data::Vertex& v3)
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
		template<class S, class T = float> std::span<T> BufferView(const GLTF::Accessor& accessor)
		{
			auto bufferView = gltf.bufferViews[accessor.bufferView];
			auto start = buffers[bufferView.buffer].data() + bufferView.byteOffset + accessor.byteOffset;
			auto size = accessor.count * glm::max<int>(bufferView.byteStride, sizeof(S)); // if bytestride is not 0, it should not be less that sizeof(T)
			return std::span<T>((T*)start, size / sizeof(T));
		}
		fs::path SrcPath(std::string uri) { return srcFolder / uri; }
	private:
		GLTF::GLTF gltf;
		fs::path srcFolder; // absolute
		fs::path dstFolder; // relative

		std::vector<std::vector<char>> buffers;
		std::vector<std::shared_ptr<Texture2D>> images;
		std::vector<Material> materials;
		std::vector<std::vector<Mesh>> meshes;
		Material defaultMaterial = RenderingSystem::Instance().world.AddMaterial(); // should be GLTF default

		std::shared_ptr<Components::Model> model;
	};
}