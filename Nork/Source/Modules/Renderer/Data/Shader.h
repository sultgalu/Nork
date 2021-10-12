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
		GLuint program;
	};

	struct Shader
	{
		inline Shader() : program(0) {} // for convinience. Could cause issues
		inline Shader(const ShaderResource resource)
			: program(resource.program)
		{
			uniformLocations = Utils::Shader::GetUniforms(program);
		}
		inline void Use()
		{
			glUseProgram(this->program);
		}
		inline void SetMat4(std::string name, glm::mat4& value)
		{
			auto loc = this->uniformLocations.find(name);
			if (loc != this->uniformLocations.end())
			{
				glUniformMatrix4fv((*loc).second, 1, false, (const GLfloat*)(&value));
			}
			else
			{
				int newLoc = TryGetLocation(name.c_str());
				this->uniformLocations[name] = newLoc;
				if (newLoc != -1)
				{
					if ((*loc).second != -1)
						glUniformMatrix4fv(newLoc, 1, false, (const GLfloat*)(&value));
					MetaLogger().Warning("GetUniforms didn't find uniform \"", name, "\", but the shader cached it.");
				}
				else MetaLogger().Warning("Trying to set invalid shader uniform: \"", name, "\" to \"",
					std::format("(\n\t{},{},{},{}\n\t{},{},{},{}\n\t{},{},{},{}\n\t\)",
						value[0][0], value[0][1], value[0][2], value[0][3],
						value[1][0], value[1][1], value[1][2], value[1][3],
						value[2][0], value[2][1], value[2][2], value[2][3],
						value[3][0], value[3][1], value[3][2], value[3][3]
					), "\"");
			}
		}
		inline void SetVec4(std::string name, glm::vec4& value)
		{
			auto loc = this->uniformLocations.find(name);
			if (loc != this->uniformLocations.end())
			{
				if ((*loc).second != -1)
					glUniform4f((*loc).second, value.x, value.y, value.z, value.w);
			}
			else
			{
				int newLoc = TryGetLocation(name.c_str());
				this->uniformLocations[name] = newLoc;
				if (newLoc != -1)
				{
					glUniformMatrix4fv(newLoc, 1, false, (const GLfloat*)(&value));
					MetaLogger().Warning("GetUniforms didn't find uniform \"", name, "\", but the shader cached it.");
				}
				else MetaLogger().Error("Trying to set invalid shader uniform: \"", name, "\" to \"", std::format("({},{},{},{})", value.x, value.y, value.z, value.w), "\"");
			}
		}
		inline void SetVec3(std::string name, glm::vec3& value)
		{
			auto loc = this->uniformLocations.find(name);
			if (loc != this->uniformLocations.end())
			{
				if ((*loc).second != -1)
					glUniform3f((*loc).second, value.x, value.y, value.z);
			}
			else
			{
				int newLoc = TryGetLocation(name.c_str());
				this->uniformLocations[name] = newLoc;
				if (newLoc != -1)
				{
					glUniformMatrix4fv(newLoc, 1, false, (const GLfloat*)(&value));
					MetaLogger().Warning("GetUniforms didn't find uniform \"", name, "\", but the shader cached it.");
				}
				else MetaLogger().Error("Trying to set invalid shader uniform: \"", name, "\" to \"", std::format("({},{},{})", value.x, value.y, value.z), "\"");
			}
		}
		inline void SetFloat(std::string name, float value)
		{
			auto loc = this->uniformLocations.find(name);
			if (loc != this->uniformLocations.end())
			{
				if ((*loc).second != -1)
					glUniform1f((*loc).second, value);
			}
			else
			{
				int newLoc = TryGetLocation(name.c_str());
				this->uniformLocations[name] = newLoc;
				if (newLoc != -1)
				{
					glUniformMatrix4fv(newLoc, 1, false, (const GLfloat*)(&value));
					MetaLogger().Warning("GetUniforms didn't find uniform \"", name, "\", but the shader cached it.");
				}
				else MetaLogger().Error("Trying to set invalid shader uniform: \"", name, "\" to \"", value, "\"");
			}
		}
		inline void SetInt(std::string name, int value)
		{
			auto loc = this->uniformLocations.find(name);
			if (loc != this->uniformLocations.end())
			{
				if((*loc).second != -1)
					glUniform1i((*loc).second, value);
			}
			else
			{
				int newLoc = TryGetLocation(name.c_str());
				this->uniformLocations[name] = newLoc;
				if (newLoc != -1)
				{
					glUniformMatrix4fv(newLoc, 1, false, (const GLfloat*)(&value));
					MetaLogger().Warning("GetUniforms didn't find uniform \"", name, "\", but the shader cached it.");
				}
				else MetaLogger().Error("Trying to set invalid shader uniform: \"", name, "\" to \"", value, "\"");
			}
		}

		inline GLuint GetProgram() { return program; }
		inline const auto& GetUniformMap() { return uniformLocations; }
	private:
		inline int TryGetLocation(const char* name) { return glGetUniformLocation(program, name); }
	private:
		GLuint program;
		std::unordered_map<std::string, GLint> uniformLocations;
	};
}

