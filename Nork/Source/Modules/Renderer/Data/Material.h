#pragma once
namespace Nork::Renderer::Data {
	struct Material
	{
		uint64_t baseColor;
		uint64_t normal;
		uint64_t metallicRoughness;
		float roughnessFactor = 1;
		float metallicFactor = 1;
		glm::vec3 baseColorFactor = { 1, 1, 1 };
		float padding;
	};
}