module Nork.Renderer;

namespace Nork::Renderer {
	std::vector<unsigned int> GetCubeIndices();
	std::vector<Data::Vertex> GetCubeVertices();
	std::shared_ptr<Mesh> MeshFactory::Create(const std::vector<Data::Vertex>& vertices, const std::vector<uint32_t>& indices)
	{
		auto vertexIdx = vaoWrapper.GetVertexWrapper().Add(vertices);
		auto indexIdx = vaoWrapper.GetIndexWrapper().Add(indices);
		return std::make_shared<Mesh>(vaoWrapper, vertexIdx, indexIdx, vertices.size(), indices.size());
	}
	std::shared_ptr<Mesh> MeshFactory::CreateCube()
	{
		return Create(GetCubeVertices(), GetCubeIndices());
	}
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
	static std::vector<Data::Vertex> GetCubeVertices()
	{
		auto positions = GetCubeVertexPositions();
		auto texCoords = GetCubeVertexTexCoords();
		auto normals = GetCubeVertexNormals();
		auto tangents = GetCubeVertexTangents();
		auto bitangents = GetCubeVertexBitangents();

		std::vector<Data::Vertex> vertices;
		for (int i = 0; i < positions.size() / 3; i++)
		{
			Data::Vertex vertex;
			vertex.position = glm::vec3(positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]);
			vertex.normal = glm::vec3(normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]);
			vertex.tangent = glm::vec3(tangents[i * 3], tangents[i * 3 + 1], tangents[i * 3 + 2]);
			vertex.texCoords = glm::vec2(texCoords[i * 2], texCoords[i * 2 + 1]);

			vertices.push_back(vertex);
		}
		return vertices;
	}
}