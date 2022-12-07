#pragma once

struct VkSuccess
{
    VkSuccess(std::source_location src = std::source_location::current())
        : src(src)
    {}
    std::source_location src;
};
inline bool operator==(const VkResult& res, const VkSuccess& a)
{
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("Vulkan call failed(" + std::string(std::to_string(res)) + ") " + std::to_string(a.src.line()) + ", " + a.src.function_name() + ", " + a.src.file_name());
    }
    return true;
}