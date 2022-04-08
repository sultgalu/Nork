#include "pch.h"
#include "FileUtils.h"

namespace Nork {
    void FileUtils::WriteString(const std::string& str, const std::string& path)
    {
        std::ofstream file(path);
        if (!file.is_open())
        {
            Logger::Error("Failed to open file ", path);
            return;
        }
        file << str;
    }
    void FileUtils::WriteBinary(const void* data, size_t size, const std::string& path)
    {
        std::ofstream file(path, std::ios_base::binary);
        if (!file.is_open())
        {
            Logger::Error("Failed to open file ", path);
            return;
        }
        file.write((char*)data, size);
    }
    std::string FileUtils::ReadAsString(const std::string& path)
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
    void FileUtils::ReadBinary(void* data, size_t size, const std::string& path)
    {
        std::ifstream file(path, std::ios_base::binary);
        if (!file.is_open())
        {
            Logger::Error("Failed to open file ", path);
            return;
        }
        file.read((char*)data, size);
    }
}

