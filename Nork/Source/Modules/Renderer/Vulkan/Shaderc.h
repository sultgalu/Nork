#pragma once

namespace Nork::Renderer::Vulkan {
	enum class ShaderType
	{
		None, Vertex, Fragment, Geometry, Compute, Mesh
	};
	class Shaderc
	{
	public:
		static std::vector<uint32_t> Compile(const std::string& src, ShaderType type, std::vector<std::array<std::string, 2>> defines);
	};
}