#pragma once

#include "Model/Mesh.h"
#include "Model/Material.h"

namespace Nork::Renderer
{
	struct ImageData
	{
		uint32_t width, height, channels;
		//TextureFormat format;
		std::vector<char> data;
	};
	enum class ImageFormat { JPEG, PNG, BMP };
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
		static ImageData LoadImage(std::string_view path, bool forceRGBA = false);
		static void WriteImage(const ImageData&, const std::string& path, ImageFormat);
		//static void WriteTexture(const Renderer::Texture2D& tex, const std::string& path, Renderer::ImageFormat format);
		static std::array<ImageData, 6> LoadCubemapImages(std::string dirPath, std::string extension);
		static std::string LoadShader(std::string_view path);
	};
}