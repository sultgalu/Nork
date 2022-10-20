export module Nork.Physics:SAT;

export import :World;

export namespace Nork::Physics
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
		static std::vector<std::pair<std::string, float>> GetDeltas();
	private:
		bool FacePhase(const Shape& useFaces, const Shape& useVerts, CollisionType resType);
		bool EdgePhase();
	private:
		 const Shape& shape1;
		 const Shape& shape2;
		 CollisionResult state = CollisionResult{ .depth = -std::numeric_limits<float>::max() };
	};

}
