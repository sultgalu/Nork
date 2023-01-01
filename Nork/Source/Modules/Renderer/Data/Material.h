#pragma once
namespace Nork::Renderer::Data {
	struct Material
	{
		uint32_t baseColor;
		uint32_t normal;
		uint32_t metallicRoughness;
		float roughnessFactor = 1;

		glm::vec3 baseColorFactor = { 1, 1, 1 };
		float metallicFactor = 1;

		float alphaCutoff = -1;
		glm::vec3 padding_;
	};
}