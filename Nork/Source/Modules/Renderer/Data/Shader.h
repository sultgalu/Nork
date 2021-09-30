#pragma once

#include "../Utils.h"

namespace Nork::Renderer::Data
{
	struct ShaderData
	{
		std::string source;
	};

	struct ShaderResource
	{
		const GLuint program;
	};

	struct Shader
	{
		inline Shader(ShaderResource resource) 
			: program(resource.program)
		{
			uniformLocations = Utils::Shader::GetUniforms(program);
		}
		inline void Use()
		{
			glUseProgram(this->program);
		}
		inline void SetMat4(std::string_view name, glm::mat4& value)
		{
			auto loc = this->uniformLocations.find(name);
			if (loc != this->uniformLocations.end())
			{
				glUniformMatrix4fv( (*loc).second, 1, false, (const GLfloat*)(&value));
			}
		}
		inline void SetVec4(std::string_view name, glm::vec4& value)
		{
			auto loc = this->uniformLocations.find(name);
			if (loc != this->uniformLocations.end())
			{
				glUniform4f((*loc).second, value.x, value.y, value.z, value.w);
			}
			
		}
		inline void SetVec3(std::string_view name, glm::vec3& value)
		{
			auto loc = this->uniformLocations.find(name);
			if (loc != this->uniformLocations.end())
			{
				glUniform3f((*loc).second, value.x, value.y, value.z);
			}
		}
		inline void SetFloat(std::string_view name, float value)
		{
			auto loc = this->uniformLocations.find(name);
			if (loc != this->uniformLocations.end())
			{
				glUniform1f((*loc).second, value);
			}
		}
		inline void SetInt(std::string_view name, int value)
		{
			auto loc = this->uniformLocations.find(name);
			if (loc != this->uniformLocations.end())
			{
				glUniform1i((*loc).second, value);
			}
		}

		inline GLuint GetProgram() { return program; }
		inline const auto& GetUniformMap() { return uniformLocations; }
	private:
		GLuint program;
		std::unordered_map<std::string_view, GLint> uniformLocations;
	};
}

