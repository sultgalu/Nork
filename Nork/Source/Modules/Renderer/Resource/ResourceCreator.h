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
		Format format;
		switch (data.channels)
		{
			using enum Format;
		case 3: [[likely]]
			format = RGB;
			break;
		case 4:
			format = RGBA;
			break;
		case 1:
			format = R8;
			break;
		default:
			MetaLogger().Error("Unexpected channel number here");
			format = None;
			break;
		}
		return TextureResource
		{
			.id = Create2D(
				data.width, data.height, format, data.data.data(),
				Wrap::ClampToEdge, Filter::LinearMipmapNearest,
				true, true)
		};
	}
	inline void DeleteTexture(TextureResource& resource) { glDeleteTextures(1, &resource.id); }

	extern MeshResource CreateMesh(MeshData& data);
	extern void DeleteMesh(MeshResource& resource);

	extern ShaderResource CreateShader(ShaderData& data);
	inline void DeleteShader(ShaderResource& resource) { glDeleteProgram(resource.program); }
}