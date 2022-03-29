#pragma once

#include "Objects/Shader/Shader.h"
#include "Objects/Texture/Texture.h"
#include "Model/Mesh.h"
#include "Model/Material.h"

namespace Nork::Renderer
{
	struct Image
	{
		uint32_t width, height, channels;
		TextureFormat format;
		std::vector<char> data;
	};
	struct MaterialData
	{
		std::unordered_map<TextureMap, std::string> textureMaps;
		glm::vec3 diffuse;
		float specular;
		float specularExponent;
	};
	struct MeshData
	{
		std::string meshName, materialName;
		std::vector<Data::Vertex> vertices;
		std::vector<GLuint> indices;
		MaterialData material;
	};
	class LoadUtils
	{
	public:
		static Image LoadImage(std::string_view path);
		static void WriteImage(const Image&, const std::string& path);
		static std::array<Image, 6> LoadCubemapImages(std::string dirPath, std::string extension);
		static std::string LoadShader(std::string_view path);
		static std::vector<MeshData> LoadModel(const std::string& path);
	};
}