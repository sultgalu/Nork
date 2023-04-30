#include "pch.h"
#include "FileUtils.h"

namespace Nork {
    void FileUtils::WriteString(const std::string& str, const fs::path& path)
    {
        std::ofstream file(path);
        if (!file.is_open())
        {
            Logger::Error("Failed to open file ", path);
            return;
        }
        file << str;
    }
    void FileUtils::WriteBinary(const void* data, size_t size, const fs::path& path)
    {
        std::ofstream file(path, std::ios_base::binary);
        if (!file.is_open())
        {
            Logger::Error("Failed to open file ", path);
            return;
        }
        file.write((char*)data, size);
    }
    std::string FileUtils::ReadAsString(const fs::path& path)
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
    void FileUtils::ReadBinary(void* data, size_t size, const fs::path& path, size_t offs)
    {
        std::ifstream file(path, std::ios_base::binary);
        if (!file.is_open())
        {
            Logger::Error("Failed to open file ", path);
            return;
        }
        file.seekg(offs).read((char*)data, size);
    }
}

