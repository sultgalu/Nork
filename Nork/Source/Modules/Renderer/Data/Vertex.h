#pragma once

namespace Nork::Renderer::Data {
	struct Vertex
	{
		glm::vec3 position, normal;
		glm::vec2 texCoords;
		glm::vec3 tangent;

		static const std::vector<vk::VertexInputBindingDescription> GetBindingDescription();
		static const std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions();
	};
	struct VertexSkinned : Vertex
	{
		std::array<uint32_t, 4> joints;
		std::array<float, 4> weights;

		static const std::vector<vk::VertexInputBindingDescription> GetBindingDescription();
		static const std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions();
	};
}