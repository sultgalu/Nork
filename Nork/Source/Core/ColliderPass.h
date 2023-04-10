#pragma once
#include "Modules/Renderer/RenderPass.h"
#include "Modules/Renderer/Image.h"
#include "Modules/Renderer/DeviceData.h"

namespace Nork {

namespace Vulkan = Renderer::Vulkan;

class ColliderPass : public Renderer::RenderPass
{
public:
	struct Vertex {
		glm::vec3 position;

		static const std::vector<vk::VertexInputBindingDescription> getBindingDescription()
		{
			std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1);
			bindingDescriptions[0].binding = 0;
			bindingDescriptions[0].stride = sizeof(Vertex);
			bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;

			return bindingDescriptions;
		}
		static const std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions()
		{
			std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(1);

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
			attributeDescriptions[0].offset = offsetof(Vertex, position);

			return attributeDescriptions;
		}
	};
public:
	ColliderPass(const std::shared_ptr<Renderer::Image>& target);
	void CreateGraphicsPipeline();
	void CreateRenderPass();
	void RecordCommandBuffer(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame) override;

	static ColliderPass& Instance() {
		return *instance;
	}

	void UpdateBuffers(const entt::registry& registry);
public:
	std::shared_ptr<Renderer::DeviceArrays<Vertex>> vertexBuffer;
	std::shared_ptr<Renderer::DeviceArrays<uint32_t>> indexBuffer;
	std::shared_ptr<Renderer::BufferView<Vertex>> vertices;
	std::shared_ptr<Renderer::BufferView<uint32_t>> indices;
	uint32_t drawIndexCount = 0;

	std::shared_ptr<Vulkan::RenderPass> renderPass;

	std::shared_ptr<Renderer::Image> depth;
	std::shared_ptr<Renderer::Image> color;
	std::shared_ptr<Vulkan::Framebuffer> fb;

	std::shared_ptr<Vulkan::DescriptorPool> descriptorPool;

	std::shared_ptr<Vulkan::PipelineLayout> pipelineLayout;
	std::shared_ptr<Vulkan::Pipeline> pipeline;

	bool enabled;
	bool useGlobalColliderVertices = false; // by default calculate the vertex positions with local collider + Transform
	glm::vec3 lineColor = { 0.2, 1.0, 0.3 };
	static ColliderPass* instance;
};
}

