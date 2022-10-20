module;
#include "Logger.h"
export module FileUtils;

// import std;

export namespace Nork {
    export class FileUtils
    {
    public:
        static void WriteString(const std::string& str, const std::string& path)
        {
            std::ofstream file(path);
            if (!file.is_open())
            {
                Logger::Error("Failed to open file ", path);
                return;
            }
            file << str;
        }
        static void WriteBinary(const void* data, size_t size, const std::string& path)
        {
            std::ofstream file(path, std::ios_base::binary);
            if (!file.is_open())
            {
                Logger::Error("Failed to open file ", path);
                return;
            }
            file.write((char*)data, size);
        }
        static std::string ReadAsString(const std::string& path)
        {
            std::ifstream file(path);
            if (!file.is_open())
            {
                Logger::Error("Failed to open file ", path);
                return "";
            }
            std::stringstream ss;
            ss << file.rdbuf();
            return ss.str();
        }
        static void ReadBinary(void* data, size_t size, const std::string& path)
        {
            std::ifstream file(path, std::ios_base::binary);
            if (!file.is_open())
            {
                Logger::Error("Failed to open file ", path);
                return;
            }
            file.read((char*)data, size);
        }
        template<class T>
        static std::vector<T> ReadBinary(const std::string& path, size_t size)
        {
            std::vector<T> result(size / sizeof(T));
            ReadBinary(result.data(), size, path);
            return result;
        }
    };
}