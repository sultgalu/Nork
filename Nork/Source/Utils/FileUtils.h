#pragma once

namespace Nork {
	class FileUtils
	{
	public:
		static void WriteString(const std::string& str, const fs::path& path);
		static void WriteBinary(const void* data, size_t size, const fs::path& path);
		static std::string ReadAsString(const fs::path& path);
		static void ReadBinary(void* data, size_t size, const fs::path& path, size_t offs = 0);
		template<class T>
		static void WriteBinary(const std::vector<T>& data, const fs::path& path)
		{
			WriteBinary(data.data(), data.size() * sizeof(T), path);
		}
		template<class T>
		static std::vector<T> ReadBinary(const fs::path& path, size_t size = 0, size_t offset = 0)
		{
			if (size == 0)
				size = std::filesystem::file_size(path);
			std::vector<T> result(size / sizeof(T));
			ReadBinary(result.data(), size, path, offset);
			return result;
		}
	};
}