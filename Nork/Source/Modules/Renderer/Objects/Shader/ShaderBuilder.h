#pragma once

#include "Shader.h"

namespace Nork::Renderer {
	class ShaderBuilder
	{
	public:
		ShaderBuilder& Sources(const std::unordered_map<ShaderType, std::string>& sources);
		std::shared_ptr<Shader> Create();
	private:
		bool Compile();
	private:
		GLuint handle;
		std::unordered_map<ShaderType, std::string> sources;
		std::vector<ShaderType> shaderTypes;
	};
}
