#pragma once

#include "../Data/World.h"

namespace Nork::Physics
{
	template<typename T>
	struct GPUBuffer
	{
		GPUBuffer(void* ptr, size_t size)
			:buf(std::span<T>((T*)ptr, size))
		{
		}
		void Unmap()
		{
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}

		std::span<T> buf;
	};
	class SAT
	{
	public:
		SAT(const Shape& shape1, const Shape& shape2);
		CollisionResult GetResult();
	private:
		bool FacePhase(const Shape& useFaces, const Shape& useVerts, CollisionType resType);
		bool EdgePhase(const Shape& shape1, const Shape& shape2);
	private:
		 CollisionResult state = CollisionResult{ .depth = -std::numeric_limits<float>::max() };
	};

}
