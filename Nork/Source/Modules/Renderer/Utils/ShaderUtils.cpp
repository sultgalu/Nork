#include "../Utils.h"

namespace Nork::Renderer::Utils::Shader
{
	GLenum GetTypeByString(std::string_view str)
	{
		if (str._Equal("vertex")) [[likely]]
			return GL_VERTEX_SHADER;
		else if (str._Equal("fragment"))
			return GL_FRAGMENT_SHADER;
		else if (str._Equal("geometry"))
			return GL_GEOMETRY_SHADER;
		else if (str._Equal("compute"))
			return GL_COMPUTE_SHADER;
		
		Logger::Error("ERR:: Unkown shader type ", str);
		return GL_NONE;
	}
	std::unordered_map<GLenum, std::string> ReadShaderSources(std::string_view s)
	{
		const char* label = "#type";
		size_t labelLen = strlen(label);

		std::unordered_map<GLenum, std::string> shaderSrcs;

		size_t pos = 0;
		GLenum shadType;
		do
		{ // the file must start with #type
			size_t start = pos;
			size_t eol = s.find_first_of("\r\n", pos); // idx of the end of the "#type" line

			pos += labelLen + 1; // the first idx of "vertex"
			std::string_view type = s.substr(pos, eol - pos); // must be "#type vertex" with whitespaces exactly like that.
			shadType = GetTypeByString(type);

			//size_t nextLinePos = s.find_first_not_of("\r\n", eol); // filtering out spaces
			pos = s.find(label, eol); // next idx starting with #type
			if (pos == std::string::npos) // no more shaders
			{
				shaderSrcs[shadType] = s.substr(eol);
				break;
			}
			shaderSrcs[shadType] = s.substr(eol, pos - eol); // more shaders ahead

		} while (true);
		return shaderSrcs;
	}
	GLuint GetProgramFromSource(std::string_view src)
	{
		std::unordered_map<GLenum, std::string> shaders = ReadShaderSources(src);
		std::unordered_map<GLenum, int> handles;

		/*for (auto& vals : definedVals)
		{
			macros[vals.shaderType].shaderType = vals.shaderType;

			std::string& src = shaders[vals.shaderType];
			int idx = src.find_first_of("#version");
			idx = src.find_first_of("\r\n", idx);
			for (auto& val : vals.values)
			{
				src.insert(idx, "\r\n#define " + val._Myfirst._Val + " " + val._Get_rest()._Myfirst._Val);
				macros[vals.shaderType].values.push_back(val);
			}
		}*/

		int success;
		char infoLog[512] = {};

		for (auto& s : shaders)
		{
			int handle = glCreateShader(s.first);
			const GLchar* src = s.second.c_str();
			glShaderSource(handle, 1, &src, nullptr);
			glCompileShader(handle);

			glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				Logger::Error("SHADER::COMPILATION_FAILED");
				return 0;
			}
			handles[s.first] = handle;
		}

		int program = glCreateProgram();
		for (auto& s : handles)
		{
			glAttachShader(program, s.second);
		}

		glLinkProgram(program);

		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success)
		{
			Logger::Error("SHADER::LINKING_FAILED");
			return 0;
		}

		for (auto& s : handles)
		{
			glDeleteShader(s.second);
		}

		return program;
	}
	
	std::unordered_map<std::string_view, GLint> GetUniforms(GLuint program)
	{
		std::unordered_map<std::string_view, GLint> retval;
		int count = 0;
		glUseProgram(program);
		glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);

		for (int i = 0; i < count; i++)
		{
			char buf[100];
			int nameLen = 0;
			int size = 0; // for arrays (that are not of structures)
			GLenum type;
			glGetActiveUniform(program, i, sizeof(buf), &nameLen, &size, &type, buf);
			int loc = glGetUniformLocation(program, buf);

			if (loc == -1)
			{
				Logger::Warning("Got uniform name, but couldn't read it's location: ", buf);
				continue;
			}

			retval[buf] = loc;
		}

		return retval;
	}
	std::unordered_map<std::string_view, bool> GetMacros(std::string_view str)
	{
		std::unordered_map<std::string_view, bool> result;

		const char* definition = "#define";
		size_t defLen = strlen(definition);

		size_t pos = 0;

		size_t prevLineEnd = 0;
		while (prevLineEnd != std::string::npos)
		{
			size_t lineEnd = str.find_first_of('\n', prevLineEnd + 1);
			std::string_view line = str.substr(prevLineEnd + 1, lineEnd);
			prevLineEnd = lineEnd;
			
			size_t definePos = line.find_first_of("#define");
			if (definePos == std::string::npos)
				continue;
			size_t macroPos = line.find_first_not_of(' ', definePos + sizeof("#define"));
			size_t macroLen = line.find_first_of(' ', macroPos) - macroPos;
			std::string_view macroName = line.substr(macroPos, macroLen);

			if (macroPos != std::string::npos)
			{
				size_t commentPos = line.find_first_of("//");
				if (commentPos != std::string::npos && commentPos < definePos)
				{
					result[macroName] = false;
					continue;
				}
			}
			result[macroName] = true;
		}
		return result;
	}

	// Can only set the macros present in "src"
	std::string SetMacros(std::string_view src, std::unordered_map<std::string_view, bool> macros)
	{
		std::string str = std::string(src);
		std::unordered_map<std::string_view, bool> result;

		const char* definition = "#define";
		size_t defLen = strlen(definition);
		size_t pos = 0;

		while (pos != std::string::npos)
		{
			size_t definePos = str.find_first_of("#define", pos);
			size_t macroPos = str.find_first_not_of(' ', definePos + sizeof("#define"));
			size_t macroLen = str.find_first_of(' ', macroPos) - macroPos;
			std::string_view macroName = str.substr(macroPos, macroLen);
			if (macros.contains(macroName))
			{
				bool shouldBeCommented = macros[macroName];
				size_t lineBegin = str.substr(pos, definePos).find_last_of('\n') + 1;
				size_t commentPos = str.substr(lineBegin, definePos - lineBegin).find_first_of("//");
				if (commentPos != std::string::npos && !shouldBeCommented)
					str.erase(commentPos, 2);
				else if (commentPos == std::string::npos && shouldBeCommented)
					str.insert(lineBegin, 2, '/');
			}
			pos = str.find_first_of('\n', pos);
		}
		return str;
	}

}