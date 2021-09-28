#pragma once

#include "../Data/Shader.h"
#include "../Data/Texture.h"
#include "../Data/Mesh.h"

namespace Nork::Renderer::Loaders
{
	 Data::TextureData LoadImage(std::string_view path);
	 Data::ShaderData LoadShader(std::string_view path);
	 std::vector<Data::MeshData> LoadModel(std::string path);
}