#include "Shaderc.h"
#include <shaderc/shaderc.hpp>

namespace Nork {

    // Returns GLSL shader source text after preprocessing.
    std::string preprocess_shader(const std::string& source_name,
        shaderc_shader_kind kind,
        const std::string& source)
    {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        // Like -DMY_DEFINE=1
        options.AddMacroDefinition("MY_DEFINE", "1");

        shaderc::PreprocessedSourceCompilationResult result =
            compiler.PreprocessGlsl(source, kind, source_name.c_str(), options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            Logger::Error(result.GetErrorMessage());
            return "";
        }

        return { result.cbegin(), result.cend() };
    }

    // Compiles a shader to SPIR-V assembly. Returns the assembly text
    // as a string.
    std::string compile_file_to_assembly(const std::string& source_name,
        shaderc_shader_kind kind,
        const std::string& source,
        bool optimize = false)
    {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        // Like -DMY_DEFINE=1
        options.AddMacroDefinition("MY_DEFINE", "1");
        if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);

        shaderc::AssemblyCompilationResult result = compiler.CompileGlslToSpvAssembly(
            source, kind, source_name.c_str(), options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            Logger::Error(result.GetErrorMessage());
            return "";
        }

        return { result.cbegin(), result.cend() };
    }

    // Compiles a shader to a SPIR-V binary. Returns the binary as
    // a vector of 32-bit words.
    std::vector<uint32_t> compile_file(const std::string& source_name,
        shaderc_shader_kind kind,
        const std::string& source,
        bool optimize = false)
    {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        // Like -DMY_DEFINE=1
        options.AddMacroDefinition("MY_DEFINE", "1");
        if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);

        shaderc::SpvCompilationResult module =
            compiler.CompileGlslToSpv(source, kind, source_name.c_str(), options);

        if (module.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            Logger::Error(module.GetErrorMessage());
            return std::vector<uint32_t>();
        }

        return { module.cbegin(), module.cend() };
    }

    static shaderc_shader_kind GetShaderKind(Renderer::ShaderType type)
    {
        using enum Renderer::ShaderType;
        switch (type)
        {
        case Vertex:
            return shaderc_glsl_vertex_shader;
        case Fragment:
            return shaderc_glsl_fragment_shader;
        case Geometry:
            return shaderc_glsl_geometry_shader;
        case Compute:
            return shaderc_glsl_compute_shader;
        case None:
        default:
            abort();
        }
    }

    std::vector<uint32_t> Shaderc::Compile(const std::string& src, Renderer::ShaderType type)
    {
        auto kind = GetShaderKind(type);

        // Preprocess
        auto preprocessed = preprocess_shader("shader_src", kind, src);
        // Logger::Info("Preprocessed shader: ", preprocessed);

        // // Compile
        // auto assembly = compile_file_to_assembly("shader_src", kind, src);
        // Logger::Info("SPIR-V assembly: ", assembly);
        // 
        // auto spirv = compile_file("shader_src", kind, src);
        // Logger::Info("Compiled to a binary module with ", spirv.size(), "words.");
        // 
        // // Compile with optimizing
        // auto assemblyOpt = compile_file_to_assembly("shader_src", kind, src, true);
        // Logger::Info("SPIR-V optimized assembly: ", assemblyOpt);

        auto spirvOpt = compile_file("shader_src", kind, src, true);
        Logger::Info("Compiled to an optimized binary module with ", spirvOpt.size(), "words.");
        return spirvOpt;
    }
}