#pragma once

#include "Modules/Physics/Data/World.h"
#include "Modules/Physics/Utils.h"
#include "Modules/Physics/Pipeline/GPUPipeline.h"

namespace Nork
{
	struct Polygon
	{
		Polygon(float size = 1);

		std::vector<glm::vec3> vertices;
		std::vector<std::unordered_set<uint32_t>> neighbours;

		std::vector<std::array<uint32_t, 3>> tris;
		std::vector<std::array<uint32_t, 2>> edges;

		uint32_t Add(glm::vec3&& v);
		void Remove(std::span<uint32_t> multiple);
		void Remove(uint32_t idx);
		void Disconnect(uint32_t first, uint32_t second);
		void Connect(uint32_t first, uint32_t second);
		void Scale(glm::vec3 scale);

		std::vector<std::pair<uint32_t, uint32_t>> GetEdges(const std::span<std::vector<uint32_t>>& faces);

		std::vector<uint32_t> SortFaceIdxs(const std::vector<uint32_t>& face);

		std::vector<std::vector<uint32_t>> GetFaces();
		Physics::Collider AsCollider();

		static Polygon GetCube(glm::vec3 pos = glm::vec3(0));
	};
}