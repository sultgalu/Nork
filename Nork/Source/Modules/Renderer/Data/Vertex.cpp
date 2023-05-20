#include "Vertex.h"

namespace Nork::Renderer::Data
{
const std::vector<vk::VertexInputBindingDescription> Vertex::GetBindingDescription()
{
	std::vector<vk::VertexInputBindingDescription> bindingDescriptions(2);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;
	bindingDescriptions[1].binding = 1;
	bindingDescriptions[1].stride = sizeof(uint32_t) * 2;
	bindingDescriptions[1].inputRate = vk::VertexInputRate::eInstance;

	return bindingDescriptions;
}
const std::vector<vk::VertexInputAttributeDescription> Vertex::GetAttributeDescriptions()
{
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(6);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[0].offset = offsetof(Vertex, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[1].offset = offsetof(Vertex, normal);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
	attributeDescriptions[2].offset = offsetof(Vertex, texCoords);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[3].offset = offsetof(Vertex, tangent);

	attributeDescriptions[4].binding = 1;
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].format = vk::Format::eR32Uint;
	attributeDescriptions[4].offset = 0;

	attributeDescriptions[5].binding = 1;
	attributeDescriptions[5].location = 5;
	attributeDescriptions[5].format = vk::Format::eR32Uint;
	attributeDescriptions[5].offset = sizeof(uint32_t);

	return attributeDescriptions;
}
const std::vector<vk::VertexInputBindingDescription> Data::VertexSkinned::GetBindingDescription()
{
	std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(VertexSkinned);
	bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;

	return bindingDescriptions;
}
const std::vector<vk::VertexInputAttributeDescription> Data::VertexSkinned::GetAttributeDescriptions()
{
	auto attributeDescriptions = Vertex::GetAttributeDescriptions();

	auto& descJoints = attributeDescriptions.emplace_back();
	descJoints.binding = 0;
	descJoints.location = attributeDescriptions.size() - 1;
	descJoints.format = vk::Format::eR32G32B32A32Sfloat;
	descJoints.offset = offsetof(VertexSkinned, joints);

	auto& descWeights = attributeDescriptions.emplace_back();
	descWeights.binding = 0;
	descWeights.location = attributeDescriptions.size() - 1;
	descWeights.format = vk::Format::eR32G32B32A32Sfloat;
	descWeights.offset = offsetof(VertexSkinned, weights);

	return attributeDescriptions;
}
}