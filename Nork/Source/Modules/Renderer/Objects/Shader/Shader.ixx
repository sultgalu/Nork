export module Nork.Renderer:Shader;

export import :GLObject;

export namespace Nork::Renderer {
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
		Shader(GLuint handle, std::vector<ShaderType> types)
			: GLObject(handle), shaderTypes(types)
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
		Shader& SetMat4(const std::string& name, const glm::mat4& value)
		{
			glUniformMatrix4fv(GetOrQueryUniformLocation(name), 1, false, (const GLfloat*)(&value));
			return *this;
		}
		Shader& SetVec4(const std::string& name, const glm::vec4& value)
		{
			glUniform4f(GetOrQueryUniformLocation(name), value.x, value.y, value.z, value.w);
			return *this;
		}
		Shader& SetVec2(const std::string& name, const glm::vec2& value)
		{
			glUniform2f(GetOrQueryUniformLocation(name), value.x, value.y);
			return *this;
		}
		Shader& SetVec3(const std::string& name, const glm::vec3& value)
		{
			glUniform3f(GetOrQueryUniformLocation(name), value.x, value.y, value.z);
			return *this;
		}
		Shader& SetFloat(const std::string& name, float value)
		{
			glUniform1f(GetOrQueryUniformLocation(name), value);
			return *this;
		}
		Shader& SetInt(const std::string& name, int value)
		{
			glUniform1i(GetOrQueryUniformLocation(name), value);
			return *this;
		}
		Shader& SetBindlessHandle(const std::string& name, uint64_t value)
		{
			glUniformHandleui64ARB(GetOrQueryUniformLocation(name), value);
			return *this;
		}
		GLint GetOrQueryUniformLocation(const std::string& name)
		{
			auto loc = uniformLocations.find(name);
			if (loc == uniformLocations.end())
			{
				return QueryAndSetUniformLocation(name);
			}
			return loc->second;
		}
		GLint QueryAndSetUniformLocation(const std::string& name)
		{
			auto loc = glGetUniformLocation(handle, name.c_str());
			if (loc == -1)
			{
				Logger::Error("Couldn't find uniform location for ", name);
			}
			uniformLocations[name] = loc;
			return loc;
		}
		const std::unordered_map<std::string, GLint>& UniformLocations() { return uniformLocations; }
		std::unordered_map<std::string, size_t> QueryAllUniformNamesAndTypes();

		glm::vec4 GetVec4(const std::string& name)
		{
			glm::vec4 val = glm::zero<glm::vec4>();
			glGetUniformfv(handle, GetOrQueryUniformLocation(name), &val.x);
			return val;
		}
		glm::vec3 GetVec3(const std::string& name)
		{
			glm::vec3 val = glm::zero<glm::vec3>();
			glGetUniformfv(handle, GetOrQueryUniformLocation(name), &val.x);
			return val;
		}
		glm::vec2 GetVec2(const std::string& name)
		{
			glm::vec2 val = glm::zero<glm::vec2>();
			glGetUniformfv(handle, GetOrQueryUniformLocation(name), &val.x);
			return val;
		}
		float GetFloat(const std::string& name)
		{
			float val = 0.0f;
			glGetUniformfv(handle, GetOrQueryUniformLocation(name), &val);
			return val;
		}
		int GetInt(const std::string& name)
		{
			int val = 0.0f;
			glGetUniformiv(handle, GetOrQueryUniformLocation(name), &val);
			return val;
		}
	public:
		const std::vector<ShaderType> shaderTypes;
	private:
		std::unordered_map<std::string, GLint> uniformLocations;
	private:
		GLenum GetIdentifier() override
		{
			return GL_PROGRAM;
		}
	};
}

