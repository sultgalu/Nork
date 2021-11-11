#include "pch.h"
#include "AABB.h"

namespace Nork::Physics
{
	AABB AABBTest::CalcAABB(Shape& shape)
	{
		glm::vec3 min = shape.verts[0], max = shape.verts[0];

		for (size_t i = 1; i < shape.verts.size(); i++)
		{
			if (shape.verts[i].x < min.x)
				min.x = shape.verts[i].x;
			if (shape.verts[i].y < min.y)
				min.y = shape.verts[i].y;
			if (shape.verts[i].z < min.z)
				min.z = shape.verts[i].z;

			if (shape.verts[i].x > max.x)
				max.x = shape.verts[i].x;
			if (shape.verts[i].y > max.y)
				max.y = shape.verts[i].y;
			if (shape.verts[i].z > max.z)
				max.z = shape.verts[i].z;
		}

		return AABB { .min = min, .max = max };
	}
	AABB AABBTest::GetAABB(std::span<glm::vec3> verts)
	{
		glm::vec3 min = verts[0], max = verts[0];

		for (size_t i = 1; i < verts.size(); i++)
		{
			if (verts[i].x < min.x)
				min.x = verts[i].x;
			if (verts[i].y < min.y)
				min.y = verts[i].y;
			if (verts[i].z < min.z)
				min.z = verts[i].z;

			if (verts[i].x > max.x)
				max.x = verts[i].x;
			if (verts[i].y > max.y)
				max.y = verts[i].y;
			if (verts[i].z > max.z)
				max.z = verts[i].z;
		}

		return AABB{ .min = min, .max = max };
	}
	static float delta = 0;
	float AABBTest::GetDelta()
	{
		return delta;
	}
	uint32_t AABBTest::Intersecting(AABB& aabb1, AABB& aabb2)
	{
		for (int i = 0; i < 3; i++)
		{
			if (aabb1.min[i] < aabb2.min[i])
			{
				if (aabb1.max[i] > aabb2.min[i])
					continue;
			}
			else
			{
				if (aabb1.min[i] < aabb2.max[i])
					continue;
			}

			return 0;
		}
		return 3;
	}
	static GLuint GetInputSSBO()
	{
		static bool first = true; 
		static GLuint ssbo = 0;
		if (first)
		{
			glGenBuffers(1, &ssbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
			glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
			first = false;
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		return ssbo;
	}

	static GLuint GetResultSSBO()
	{
		static bool first = true;
		static GLuint ssbo = 0;
		if (first)
		{
			glGenBuffers(1, &ssbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
			glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW); //sizeof(data) only works for statically sized C/C++ arrays.
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
			first = false;
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		return ssbo;
	}
	std::vector<uint32_t> AABBTest::GetResult(World& world)
	{
		Timer t;
		std::vector<AABB> aabbs;
		aabbs.reserve(world.shapes.size());

		for (size_t i = 0; i < world.shapes.size(); i++)
		{
			aabbs.push_back(GetAABB(world.shapes[i].verts));
		}
		std::vector<uint32_t> results(world.shapes.size() * world.shapes.size(), 69);
		auto res = GetResultSSBO();
		glBufferData(GL_SHADER_STORAGE_BUFFER, results.size() * sizeof(uint32_t), results.data(), GL_DYNAMIC_DRAW);
		auto input = GetInputSSBO();
		glBufferData(GL_SHADER_STORAGE_BUFFER, aabbs.size() * sizeof(AABB), aabbs.data(), GL_DYNAMIC_DRAW);
		
		// calc
		glDispatchCompute(world.shapes.size(), world.shapes.size(), 1);

		res = GetResultSSBO();
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, results.size() * sizeof(uint32_t), results.data());
		delta = t.Elapsed();
		return results;
	}

	std::vector<std::pair<uint32_t, uint32_t>> AABBTest::GetResult2(World& world)
	{
		Timer t;
		std::vector<AABB> aabbs;
		aabbs.reserve(world.shapes.size());
		std::vector<std::pair<uint32_t, uint32_t>> results;

		for (size_t i = 0; i < world.shapes.size(); i++)
		{
			aabbs.push_back(GetAABB(world.shapes[i].verts));
		}
		for (size_t i = 0; i < aabbs.size(); i++)
		{
			for (size_t j = i + 1; j < aabbs.size(); j++)
			{
				if (Intersecting(aabbs[i], aabbs[j]))
					results.push_back(std::pair(i, j));
			}
		}
		delta = t.Elapsed();
		return results;
	}
}