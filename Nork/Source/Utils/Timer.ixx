export module Nork.Utils:Timer;

export namespace Nork
{
	class Timer
	{
	public:
		Timer(float offsetMillis)
		{
			start = std::chrono::high_resolution_clock::now();
			start += std::chrono::nanoseconds((uint64_t)(offsetMillis * 1000));
		}
		Timer()
		{
			start = std::chrono::high_resolution_clock::now();
		}
		void Restart()
		{
			start = std::chrono::high_resolution_clock::now();
		}
		float Reset()
		{
			float elapsed = Elapsed();
			start = std::chrono::high_resolution_clock::now();
			return elapsed;
		}
		float ElapsedSeconds() const
		{
			return Elapsed() * 0.001f;
		}
		float Elapsed() const
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count() * 0.001f * 0.001f;
		}
	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> start;
		inline static std::chrono::time_point<std::chrono::high_resolution_clock> startShared = std::chrono::high_resolution_clock::now();
	};
}
