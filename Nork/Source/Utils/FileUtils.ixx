export module Nork.Utils:FileUtils;

export namespace Nork {
	class FileUtils
	{
	public:
		static void WriteString(const std::string& str, const std::string& path);
		static void WriteBinary(const void* data, size_t size, const std::string& path);
		static std::string ReadAsString(const std::string& path);
		static void ReadBinary(void* data, size_t size, const std::string& path);
		template<class T>
		static std::vector<T> ReadBinary(const std::string& path, size_t size)
		{
			std::vector<T> result(size / sizeof(T));
			ReadBinary(result.data(), size, path);
			return result;
		}
	};
}