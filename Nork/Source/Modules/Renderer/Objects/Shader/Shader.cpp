#include "pch.h"
#include "Shader.h"

namespace Nork::Renderer {
	Shader& Shader::Compile(std::unordered_map<ShaderType, std::string> sources)
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

		return *this;
	}
}