#include "ShaderBuilder.h"
#include "../GLManager.h"

namespace Nork::Renderer {
	ShaderBuilder& ShaderBuilder::Sources(const std::unordered_map<ShaderType, std::string>& sources)
	{
		this->sources = sources;
		shaderTypes.clear();
		for (auto& pair : sources)
		{
			shaderTypes.push_back(pair.first);
		}
		return *this;
	}
	std::shared_ptr<Shader> ShaderBuilder::Create()
	{
		handle = glCreateProgram();
		Compile();
		Logger::Info("Created shader ", handle);
		auto shader = std::make_shared<Shader>(handle, shaderTypes);
		GLManager::Get().shaders[shader->GetHandle()] = shader;
		return shader;
	}
	void ShaderBuilder::Compile()
	{
		std::unordered_map<GLenum, int> handles;

		int success;
		char infoLog[512] = {};

		for (auto& s : sources)
		{
			GLenum type = std::to_underlying(s.first);
			int handle = glCreateShader(type);
			const GLchar* src = s.second.c_str();
			glShaderSource(handle, 1, &src, nullptr);
			glCompileShader(handle);

			glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				Logger::Error("SHADER::COMPILATION_FAILED");
				glGetShaderInfoLog(handle, 512, NULL, infoLog);
				Logger::Error(infoLog);
				std::abort();
			}
			handles[type] = handle;
		}

		for (auto& s : handles)
		{
			glAttachShader(handle, s.second);
		}

		glLinkProgram(handle);

		glGetProgramiv(handle, GL_LINK_STATUS, &success);
		if (!success)
		{
			Logger::Error("SHADER::LINKING_FAILED");
			glGetProgramInfoLog(handle, 512, NULL, infoLog);
			Logger::Error(infoLog);
		}

		for (auto& s : handles)
		{
			glDeleteShader(s.second);
		}
	}
}
