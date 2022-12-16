#pragma once

namespace Nork {
	enum class ShaderType
	{
		None, Vertex, Fragment, Geometry, Compute
	};
	class Shaderc
	{
	public:
		static std::vector<uint32_t> Compile(const std::string& src, ShaderType type, std::vector<std::array<std::string, 2>> defines);
	};
}