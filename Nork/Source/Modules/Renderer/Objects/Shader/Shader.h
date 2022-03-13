#pragma once
#include "../GLObject.h"

namespace Nork::Renderer {
	enum class ShaderType : GLenum
	{
		None = GL_NONE,
		Vertex = GL_VERTEX_SHADER,
		Fragment = GL_FRAGMENT_SHADER,
		Geometry = GL_GEOMETRY_SHADER,
		Compute = GL_COMPUTE_SHADER,
	};

	class Shader : public GLObject
	{
	public:
		Shader(GLuint handle)
			: GLObject(handle)
		{}
		~Shader()
		{
			Logger::Info("Deleting shader ", handle, ".");
			glDeleteProgram(handle);
		}
		Shader& Use()
		{
			glUseProgram(handle);
			return *this;
		}
		Shader& GetUniformLocationAndSet(const char* name)
		{
			uniformLocations[name] = GetUniformLocation(name);
			if (uniformLocations[name] == -1)
			{
				Logger::Error("Couldn't find uniform location for ", name);
			}
			return *this;
		}
		Shader& SetMat4(std::string name, const glm::mat4& value)
		{
			if (!uniformLocations.contains(name))
			{
				GetUniformLocationAndSet(name.c_str());
			}
			glUniformMatrix4fv(uniformLocations[name], 1, false, (const GLfloat*)(&value));
			return *this;
		}
		Shader& SetVec4(std::string name, const glm::vec4& value)
		{
			if (!uniformLocations.contains(name))
			{
				GetUniformLocationAndSet(name.c_str());
			}
			glUniform4f(uniformLocations[name], value.x, value.y, value.z, value.w);
			return *this;
		}
		Shader& SetVec2(std::string name, const glm::vec2& value)
		{
			if (!uniformLocations.contains(name))
			{
				GetUniformLocationAndSet(name.c_str());
			}
			glUniform2f(uniformLocations[name], value.x, value.y);
			return *this;
		}
		Shader& SetVec3(std::string name, const glm::vec3& value)
		{
			if (!uniformLocations.contains(name))
			{
				GetUniformLocationAndSet(name.c_str());
			}
			glUniform3f(uniformLocations[name], value.x, value.y, value.z);
			return *this;
		}
		Shader& SetFloat(std::string name, float value)
		{
			if (!uniformLocations.contains(name))
			{
				GetUniformLocationAndSet(name.c_str());
			}
			glUniform1f(uniformLocations[name], value);
			return *this;
		}
		Shader& SetInt(std::string name, int value)
		{
			if (!uniformLocations.contains(name))
			{
				GetUniformLocationAndSet(name.c_str());
			}
			glUniform1i(uniformLocations[name], value);
			return *this;
		}
		GLint GetUniformLocation(const char* name) { return glGetUniformLocation(handle, name); }
	private:
		std::unordered_map<std::string, GLint> uniformLocations;
	private:
		GLenum GetIdentifier() override
		{
			return GL_PROGRAM;
		}
	};
}

