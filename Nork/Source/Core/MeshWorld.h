#pragma once

namespace Nork
{
	template<typename Vertex>
	struct MeshWorld
	{
		std::vector<Vertex> vertices;
		std::vector<std::set<uint32_t>> neighbours;

		std::vector<std::array<uint32_t, 3>> triangleIndices;
		std::vector<std::array<uint32_t, 2>> edgeIndices;

		inline uint32_t Add(Vertex v)
		{
			vertices.push_back(v);
			neighbours.push_back({});
			return vertices.size() - 1;
		}
		inline void Connect(uint32_t first, uint32_t second)
		{
			if (first >= neighbours.size() || second >= neighbours.size())
			{
				Logger::Error("invalid vertex index: ", first, " or ", second, ". There are only ",
					vertices.size(), " number of vertices, and ", neighbours.size(), " should equal this.");
				return;
			}
			auto& n1 = neighbours[first];
			auto& n2 = neighbours[second];
			if (n1.contains(second))
			{
				Logger::Error(first, " and ", second, " are already neighbours.");
				return;
			}

			for (uint32_t third : n1)
			{
				if (n2.contains(third)) // find common neighbours
				{
					triangleIndices.push_back({ first, second, third });
				}
			}

			n1.insert(second);
			n2.insert(first);
			edgeIndices.push_back({ second, first });
		}
		static MeshWorld<Vertex> GetCube()
		{
			MeshWorld<Vertex> meshes;
			meshes.Add(Vertex({ -1, -1, -1 }));
			meshes.Add(Vertex({ 1, -1, -1 }));
			meshes.Add(Vertex({ 1, 1, -1 }));
			meshes.Add(Vertex({ -1, 1, -1 }));

			meshes.Add(Vertex({ -1, -1, 1 }));
			meshes.Add(Vertex({ 1, -1, 1 }));
			meshes.Add(Vertex({ 1, 1, 1 }));
			meshes.Add(Vertex({ -1, 1, 1 }));

			// Front
			meshes.Connect(0, 1);
			meshes.Connect(1, 2);
			meshes.Connect(2, 3);
			meshes.Connect(3, 0);
			meshes.Connect(0, 2);
			// Right
			meshes.Connect(1, 5);
			meshes.Connect(5, 6);
			meshes.Connect(6, 2);
			meshes.Connect(1, 6);
			// Left
			meshes.Connect(0, 4);
			meshes.Connect(4, 7);
			meshes.Connect(7, 3);
			meshes.Connect(0, 7);
			// Top
			meshes.Connect(7, 6);
			meshes.Connect(3, 6);
			// Bottom
			meshes.Connect(4, 5);
			meshes.Connect(0, 5);
			// Back
			meshes.Connect(4, 6);
			return meshes;
		}
	};

	template<typename Vertex>
	struct MeshSubWorld
	{
		MeshWorld<Vertex>& world;
		uint32_t begin, end;

		std::vector<std::array<uint32_t, 3>> triangleIndices;
		std::vector<std::array<uint32_t, 2>> edgeIndices;
	};
}