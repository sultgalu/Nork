#pragma once

#include "../Data/Texture.h"
#include "../Data/Mesh.h"
#include "../Data/Shader.h"
#include "../Utils.h"

namespace Nork::Renderer::Resource
{
	using namespace Data;
	inline TextureResource CreateTexture(TextureData& data)
	{
		using namespace Utils::Texture;
		return TextureResource
		{
			.id = Create2D(data.data.data(),
				data.width, data.height, data.channels,
				Wrap::ClampToEdge, Filter::LinearMipmapNearest,
				true, true)
		};
	}
	inline void DeleteTexture(TextureResource& resource) { glDeleteTextures(1, &resource.id); }

	extern MeshResource CreateMesh(MeshData& data);
	extern void DeleteMesh(MeshResource& resource);

	inline ShaderResource CreateShader(ShaderData& data) { return ShaderResource{ .program = Utils::Shader::GetProgramFromSource(data.source) }; }
	inline void DeleteShader(ShaderResource& resource) { glDeleteProgram(resource.program); }
}