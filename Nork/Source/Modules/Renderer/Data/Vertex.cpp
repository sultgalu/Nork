#include "Vertex.h"

namespace Nork::Renderer::Data
{
const std::vector<vk::VertexInputBindingDescription> Vertex::getBindingDescription()
{
	std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;

	return bindingDescriptions;
}
const std::vector<vk::VertexInputAttributeDescription> Vertex::getAttributeDescriptions()
{
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(4);

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

	return attributeDescriptions;
}
}