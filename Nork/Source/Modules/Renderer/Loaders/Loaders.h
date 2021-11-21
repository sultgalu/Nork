#pragma once

#include "../Data/Shader.h"
#include "../Data/Texture.h"
#include "../Data/Mesh.h"

namespace Nork::Renderer::Loaders
{
	 TextureData LoadImage(std::string_view path);
	 std::array<TextureData, 6> LoadCubemapImages(std::string dirPath, std::string extension);
	 ShaderData LoadShader(std::string_view path);
	 std::vector<MeshData> LoadModel(std::string path);
}