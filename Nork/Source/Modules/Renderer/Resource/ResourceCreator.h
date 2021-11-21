#pragma once

#include "../Data/Texture.h"
#include "../Data/Mesh.h"
#include "../Data/Shader.h"
#include "../Utils.h"

namespace Nork::Renderer
{
	extern MeshResource CreateMesh(MeshData& data);
	extern void DeleteMesh(MeshResource& resource);

	extern ShaderResource CreateShader(ShaderData& data);
	inline void DeleteShader(ShaderResource& resource) { glDeleteProgram(resource.program); }

	TextureResource CreateCubemap(std::array<TextureData, 6>& datas, Utils::Texture::TextureParams params = Utils::Texture::TextureParams());
	TextureResource CreateTexture(TextureData& data, Utils::Texture::TextureParams params = Utils::Texture::TextureParams());
	inline void DeleteTexture(TextureResource& resource) { glDeleteTextures(1, &resource.id); }
}