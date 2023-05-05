#pragma once
namespace Nork::Renderer::Data {
	struct Material
	{
		uint16_t baseColor;
		uint16_t normal;
		uint16_t metallicRoughness;
		uint16_t occlusion;
		float roughnessFactor = 1;
		float metallicFactor = 1;

		glm::vec3 emissiveFactor = { 0, 0, 0 };
		uint32_t emissive; // union emissive and baseColorFactor (they are never used together) -> save 16 bytes

		glm::vec4 baseColorFactor = { 1, 1, 1, 1 };

		float alphaCutoff = -1;
		float padding[3];
	};
}