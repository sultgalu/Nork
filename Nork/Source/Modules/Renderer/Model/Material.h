#pragma once
#include "../Objects/Texture/Texture.h"
#include "../Objects/Texture/Texture.h"

namespace Nork::Renderer {
	enum class TextureMap: uint8_t
	{
		Diffuse = 0, Normal, Roughness, Reflection, COUNT
	};

	struct Material
	{
		std::unordered_map<TextureMap, std::shared_ptr<Texture2D>> textureMaps;
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
	};
}