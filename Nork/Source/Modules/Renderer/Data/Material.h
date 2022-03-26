#pragma once
namespace Nork::Renderer::Data {
	struct Material
	{
		uint64_t diffuseMap;
		uint64_t normalsMap;
		uint64_t roughnessMap;
		uint64_t reflectMap;
		glm::vec3 diffuse;
		float specular;
		float specularExponent;
		float padding[3];
	};
}