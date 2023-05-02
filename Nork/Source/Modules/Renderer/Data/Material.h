#pragma once
namespace Nork::Renderer::Data {
	struct Material
	{
		uint32_t baseColor;
		uint32_t normal;
		uint32_t metallicRoughness;
		uint32_t occlusion;

		glm::vec4 baseColorFactor = { 1, 1, 1, 1 };

		float roughnessFactor = 1;
		float metallicFactor = 1;
		float alphaCutoff = -1;
		float padding_;
	};
}