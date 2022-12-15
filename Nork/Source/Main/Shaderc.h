#pragma once

#include "Modules/Renderer/Objects/Shader/Shader.h"

namespace Nork {
	class Shaderc
	{
	public:
		static std::vector<uint32_t> Compile(const std::string& src, Renderer::ShaderType type, std::vector<std::array<std::string, 2>> defines);
	};
}