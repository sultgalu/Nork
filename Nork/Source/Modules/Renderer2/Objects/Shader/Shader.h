#pragma once
#include "../GLObject.h"

namespace Nork::Renderer2 {
	enum class ShaderType: GLenum
	{
		Vertex = GL_VERTEX_SHADER,
		Fragment = GL_FRAGMENT_SHADER,
		Geometry = GL_GEOMETRY_SHADER,
		Compute = GL_COMPUTE_SHADER,
	};

	class Shader : GLObject
	{
	public:
		Shader& Compile(std::unordered_map<ShaderType, std::string> sources);
		Shader& Create()
		{
			handle = glCreateProgram();
			Logger::Info("Created shader ", handle);
			return *this;
		}
		void Destroy()
		{
			Logger::Info("Deleting shader ", handle, ".");
			glDeleteProgram(handle);
		}
		inline void Use()
		{
			glUseProgram(handle);
		}
		inline void GetUniformLocationAndSet(const char* name)
		{
			uniformLocations[name] = GetUniformLocation(name);
			if (uniformLocations[name] == -1)
			{
				Logger::Error("Couldn't find uniform location for ", name);
			}
		}
		inline void SetMat4(std::string name, const glm::mat4& value)
		{
			if (!uniformLocations.contains(name))
			{
				GetUniformLocationAndSet(name.c_str());
			}
			glUniformMatrix4fv(uniformLocations[name], 1, false, (const GLfloat*)(&value));
		}
		inline void SetVec4(std::string name, const glm::vec4& value)
		{
			if (!uniformLocations.contains(name))
			{
				GetUniformLocationAndSet(name.c_str());
			}
			glUniform4f(uniformLocations[name], value.x, value.y, value.z, value.w);
		}
		inline void SetVec2(std::string name, const glm::vec2& value)
		{
			if (!uniformLocations.contains(name))
			{
				GetUniformLocationAndSet(name.c_str());
			}
			glUniform2f(uniformLocations[name], value.x, value.y);
		}
		inline void SetVec3(std::string name, const glm::vec3& value)
		{
			if (!uniformLocations.contains(name))
			{
				GetUniformLocationAndSet(name.c_str());
			}
			glUniform3f(uniformLocations[name], value.x, value.y, value.z);
		}
		inline void SetFloat(std::string name, float value)
		{
			if (!uniformLocations.contains(name))
			{
				GetUniformLocationAndSet(name.c_str());
			}
			glUniform1f(uniformLocations[name], value);
		}
		inline void SetInt(std::string name, int value)
		{
			if (!uniformLocations.contains(name))
			{
				GetUniformLocationAndSet(name.c_str());
			}
			glUniform1i(uniformLocations[name], value);
		}
		inline int GetUniformLocation(const char* name) { return glGetUniformLocation(handle, name); }
	private:
		std::unordered_map<std::string, GLint> uniformLocations;
	};
}

