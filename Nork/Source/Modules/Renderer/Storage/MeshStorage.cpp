#include "pch.h"
#include "MeshStorage.h"

#include "../Objects/Buffer/BufferBuilder.h"
#include "../Objects/VertexArray/VertexArrayBuilder.h"

namespace Nork::Renderer {
	namespace {
		static std::vector<unsigned int> GetCubeIndices()
		{
			std::vector<unsigned int> indices;
			indices.reserve(6 * 6);

			auto addFaceCCw = [&](int a, int b, int c, int d)
			{
				indices.push_back(a);
				indices.push_back(b);
				indices.push_back(d);

				indices.push_back(d);
				indices.push_back(b);
				indices.push_back(c);
			};

			// FACE #1 (front)
			addFaceCCw(0, 1, 2, 3);
			// FACE #4 (right)
			addFaceCCw(4, 5, 6, 7);
			// FACE #2 (left)
			addFaceCCw(8, 9, 10, 11);
			// FACE #5 (top)
			addFaceCCw(12, 13, 14, 15);
			// FACE #3 (bottom)
			addFaceCCw(16, 17, 18, 19);
			// FACE #6 (back)
			addFaceCCw(20, 21, 22, 23);

			return indices;
		}
		static std::vector<float> GetCubeVertexPositions()
		{
			std::vector<float> vertices;
			vertices.reserve(6 * 4 * 3);

			auto addPoint = [&](float x, float y, float z)
			{
				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);
			};

			float size = 1.0f;

			// FACE #1 (FRONT)
			addPoint(-size, -size, size);
			addPoint(size, -size, size);
			addPoint(size, size, size);
			addPoint(-size, size, size);

			// FACE #2 (RIGHT)
			addPoint(size, -size, size);
			addPoint(size, -size, -size);
			addPoint(size, size, -size);
			addPoint(size, size, size);

			// FACE #3 (LEFT)
			addPoint(-size, -size, -size);
			addPoint(-size, -size, size);
			addPoint(-size, size, size);
			addPoint(-size, size, -size);

			// FACE #4 (TOP)
			addPoint(-size, size, size);
			addPoint(size, size, size);
			addPoint(size, size, -size);
			addPoint(-size, size, -size);

			// FACE #5 (BOTTOM)
			addPoint(-size, -size, -size);
			addPoint(size, -size, -size);
			addPoint(size, -size, size);
			addPoint(-size, -size, size);

			// FACE #6 (BACK)
			addPoint(size, -size, -size);
			addPoint(-size, -size, -size);
			addPoint(-size, size, -size);
			addPoint(size, size, -size);

			return vertices;
		}
		static std::vector<float> GetCubeVertexPositions8()
		{
			std::vector<float> vertices;
			vertices.reserve(6 * 4 * 3);

			auto addPoint = [&](float x, float y, float z)
			{
				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);
			};

			float size = 1.0f;

			// FACE #1 (FRONT)
			addPoint(-size, -size, size);
			addPoint(size, -size, size);
			addPoint(size, size, size);
			addPoint(-size, size, size);

			// FACE #6 (BACK)
			addPoint(size, -size, -size);
			addPoint(-size, -size, -size);
			addPoint(-size, size, -size);
			addPoint(size, size, -size);

			return vertices;
		}
		static std::vector<float> GetCubeVertexTexCoords()
		{
			std::vector<float> texCoords;
			texCoords.reserve(6 * 4 * 2);

			auto addUV = [&](float x, float y)
			{
				texCoords.push_back(x);
				texCoords.push_back(y);
			};

			for (int i = 0; i < 6; i++)
			{
				addUV(0, 0);
				addUV(1, 0);
				addUV(1, 1);
				addUV(0, 1);
			}

			return texCoords;
		}
		static std::vector<float> GetCubeVertexNormals()
		{
			std::vector<float> normals;
			normals.reserve(6 * 4 * 3);

			auto addNormal = [&](float x, float y, float z)
			{
				normals.push_back(x);
				normals.push_back(y);
				normals.push_back(z);
			};

			// FACE #1 (FRONT)
			for (int i = 0; i < 4; i++)
				addNormal(0, 0, 1);
			// FACE #2 (RIGHT)
			for (int i = 0; i < 4; i++)
				addNormal(1, 0, 0);
			// FACE #3 (LEFT)
			for (int i = 0; i < 4; i++)
				addNormal(-1, 0, 0);
			// FACE #4 (TOP)
			for (int i = 0; i < 4; i++)
				addNormal(0, 1, 0);
			// FACE #5 (BOTTOM)
			for (int i = 0; i < 4; i++)
				addNormal(0, -1, 0);
			// FACE #6 (BACK)
			for (int i = 0; i < 4; i++)
				addNormal(0, 0, -1);

			return normals;
		}
		static std::vector<float> GetCubeVertexTangents()
		{
			std::vector<float> tangents;
			tangents.reserve(6 * 4 * 3);

			auto addTangent = [&](float x, float y, float z)
			{
				tangents.push_back(x);
				tangents.push_back(y);
				tangents.push_back(z);
			};

			// FACE #1 (FRONT)
			for (int i = 0; i < 4; i++)
				addTangent(0, 1, 0);
			// FACE #2 (RIGHT)
			for (int i = 0; i < 4; i++)
				addTangent(0, 1, 0);
			// FACE #3 (LEFT)
			for (int i = 0; i < 4; i++)
				addTangent(0, 1, 0);
			// FACE #4 (TOP)
			for (int i = 0; i < 4; i++)
				addTangent(0, 0, -1);
			// FACE #5 (BOTTOM)
			for (int i = 0; i < 4; i++)
				addTangent(0, 0, 1);
			// FACE #6 (BACK)
			for (int i = 0; i < 4; i++)
				addTangent(0, 1, 0);

			return tangents;
		}
		static std::vector<float> GetCubeVertexBitangents()
		{
			std::vector<float> bitangents;
			bitangents.reserve(6 * 4 * 3);

			auto addNormal = [&](float x, float y, float z)
			{
				bitangents.push_back(x);
				bitangents.push_back(y);
				bitangents.push_back(z);
			};

			// FACE #1 (FRONT)
			for (int i = 0; i < 4; i++)
				addNormal(-1, 0, 0);
			// FACE #2 (RIGHT)
			for (int i = 0; i < 4; i++)
				addNormal(0, 1, 0);
			// FACE #3 (LEFT)
			for (int i = 0; i < 4; i++)
				addNormal(0, 1, 0);
			// FACE #4 (TOP)
			for (int i = 0; i < 4; i++)
				addNormal(0, 0, -1);
			// FACE #5 (BOTTOM)
			for (int i = 0; i < 4; i++)
				addNormal(0, 0, 1);
			// FACE #6 (BACK)
			for (int i = 0; i < 4; i++)
				addNormal(0, 1, 0);

			return bitangents;
		}
	}
	static std::vector<Model::Vertex> GetCubeVertices()
	{
		auto positions = GetCubeVertexPositions();
		auto texCoords = GetCubeVertexTexCoords();
		auto normals = GetCubeVertexNormals();
		auto tangents = GetCubeVertexTangents();
		auto bitangents = GetCubeVertexBitangents();

		std::vector<Model::Vertex> vertices;
		for (int i = 0; i < positions.size() / 3; i++)
		{
			Model::Vertex vertex;
			vertex.position = glm::vec3(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
			vertex.normal = glm::vec3(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
			vertex.tangent = glm::vec3(tangents[i * 3], tangents[i * 3 + 1], tangents[i * 3 + 2]);
			vertex.texCoords = glm::vec2(texCoords[i * 2], texCoords[i * 2 + 1]);

			vertices.push_back(vertex);
		}
		return vertices;
	}
	MeshStorage::MeshStorage(size_t initialVboSize, size_t initialIboSize)
	{
		auto vbo = BufferBuilder().Target(BufferTarget::Vertex).Usage(BufferUsage::StaticDraw).Data(nullptr, initialVboSize).Create();
		auto ibo = BufferBuilder().Target(BufferTarget::Index).Usage(BufferUsage::StaticDraw).Data(nullptr, initialIboSize).Create();
		vao = VertexArrayBuilder().VBO(vbo).IBO(ibo).Attributes({ 3, 3, 2, 3 }).Create();
	}

	std::shared_ptr<Mesh> MeshStorage::AddCube()
	{
		return Add(GetCubeVertices(), GetCubeIndices());
	}

	std::vector<std::shared_ptr<Mesh>> MeshStorage::Add(const std::vector<std::pair<const std::vector<Model::Vertex>&, const std::vector<uint32_t>&>>& verticiesIndicies)
	{
		size_t vertCount = 0, indCount = 0;
		for (auto& [verts, inds] : verticiesIndicies)
		{
			vertCount += verts.size();
			indCount += inds.size();
		}
		vao->GetVBO()->Bind().Reserve(vao->GetVBO()->GetSize() + vertCount * sizeof(Model::Vertex));
		vao->GetIBO()->Bind().Reserve(vao->GetIBO()->GetSize() + indCount * sizeof(uint32_t));

		std::vector<std::shared_ptr<Mesh>> meshes;
		for (auto& [verts, inds] : verticiesIndicies)
		{
			meshes.push_back(Add(verts, inds));
		}
		return meshes;
	}

	std::shared_ptr<Mesh> MeshStorage::Add(const std::vector<Model::Vertex>& vertices, const std::vector<uint32_t> indices)
	{
		auto vbo = vao->GetVBO();
		auto ibo = vao->GetIBO();
		Range range;
		range.vertexOffs = vbo->GetCount<Model::Vertex>();
		range.indexOffs = ibo->GetCount<uint32_t>();
		range.vertexCount = vertices.size();
		range.indexCount = indices.size();

		vbo->Bind().Append(vertices);
		ibo->Bind().Append(indices);

		ranges.push_back(range);
		auto mesh = std::make_shared<Mesh>(ranges.size() - 1);
		meshes.push_back(mesh);
		return mesh;
	}
	void MeshStorage::FreeObsoleteData(bool shrinkToFit)
	{
		std::vector<Range> eraseRanges(1);
		uint32_t vertexShift = 0, indexShift = 0;
		// merge adjacent non-expired ranges
		for (auto& mesh : meshes)
		{
			auto& range = ranges[mesh->storageIdx];

			if (mesh.use_count() > 1)
			{ // start next range if current is not empty
				if (eraseRanges.back().vertexCount > 0 || eraseRanges.back().indexCount > 0)
				{ 
					eraseRanges.push_back(Range());
				}

				range.vertexOffs -= vertexShift;
				range.indexOffs -= indexShift;
			}
			else
			{ // obsolete
				if (eraseRanges.back().vertexOffs == 0 && eraseRanges.back().indexOffs == 0)
				{ // range starts from here
					eraseRanges.back() = range;
				}
				else
				{ // append range
					eraseRanges.back().vertexCount += range.vertexCount;
					eraseRanges.back().indexCount += range.indexCount;
				}

				vertexShift += range.vertexCount;
				indexShift += range.indexCount;
			}
		}

		std::vector<size_t> rangeIdxsToErase;
		for (int i = (int)meshes.size() - 1; i >= 0; i--)
		{
			if (meshes[i].use_count() <= 1)
			{
				rangeIdxsToErase.push_back(meshes[i]->storageIdx);
				meshes.erase(meshes.begin() + i);
			}
		}
		std::sort(rangeIdxsToErase.begin(), rangeIdxsToErase.end(), std::greater<size_t>());
		for (auto& idx : rangeIdxsToErase)
		{
			ranges.erase(ranges.begin() + idx);
		}

		std::vector<std::pair<size_t, size_t>> vertexRanges, indexRanges;
		std::transform(eraseRanges.begin(), eraseRanges.end(), std::back_inserter(vertexRanges),
			[](Range r) -> std::pair<size_t, size_t>
			{
				return { r.vertexOffs *= sizeof(Model::Vertex), r.vertexCount *= sizeof(Model::Vertex) };
			}); 
		std::transform(eraseRanges.begin(), eraseRanges.end(), std::back_inserter(indexRanges),
			[](Range r) -> std::pair<size_t, size_t>
			{
				return { r.indexOffs *= sizeof(uint32_t), r.indexCount *= sizeof(uint32_t) };
			});

		vao->GetVBO()->Bind().Erase(vertexRanges);
		vao->GetVBO()->Bind().Erase(indexRanges);
		if (shrinkToFit)
		{
			vao->GetVBO()->ShrinkToFit();
			vao->GetIBO()->ShrinkToFit();
		}
	}
}

