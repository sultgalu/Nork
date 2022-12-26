#pragma once

#include "Vulkan/CommandBuffer.h"
#include "Vulkan/SwapChain.h"
#include "Core/InputState.h"
#include "Vulkan/Shaderc.h"
#include "Vulkan/Image.h"
#include "LoadUtils.h"
#include "Vulkan/Semaphore.h"
#include "MemoryAllocator.h"
#include "MemoryTransfer.h"

namespace Nork::Renderer {
	inline static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	inline static constexpr uint32_t MAX_IMG_ARR_SIZE = 10;
	inline static constexpr uint32_t DRAW_BATCH_COUNT = 5;
	inline static constexpr uint32_t INSTANCE_COUNT = 2;
	inline static constexpr uint32_t DRAW_COUNT = DRAW_BATCH_COUNT * INSTANCE_COUNT;
	inline static constexpr uint32_t INVALID_FRAME_IDX = UINT_MAX;

	struct SSBO
	{
		glm::mat4 model;
	};
	struct Vertex
	{
		using Self = Vertex;
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;
		static const std::array<vk::VertexInputAttributeDescription, 3>& getAttributeDescriptions()
		{
			static auto val = getAttributeDescriptions_();
			return val;
		}
		static const vk::VertexInputBindingDescription& getBindingDescription()
		{
			static auto val = getBindingDescription_();
			return val;
		}
	private:
		static const vk::VertexInputBindingDescription getBindingDescription_()
		{
			vk::VertexInputBindingDescription bindingDescription;
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = vk::VertexInputRate::eVertex;

			return bindingDescription;
		}
		static const std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions_()
		{
			std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

			return attributeDescriptions;
		}
	};

	std::vector<vk::DescriptorPoolSize> DescriptorCounts(const std::vector<std::shared_ptr<Vulkan::DescriptorSetLayout>>& layouts)
	{
		std::vector<vk::DescriptorPoolSize> result;
		for (auto& layout : layouts)
			for (auto& binding : layout->createInfo.bindings)
				result.push_back({ binding.descriptorType, binding.descriptorCount });
		return result;
	}
	
	class State
	{
	public:
		State()
		{
			commandPool = std::make_shared<Vulkan::CommandPool>();

			vertexBuffer = Buffer::Create(
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
				sizeof(vertices[0]) * vertices.size(), vk::MemoryPropertyFlagBits::eDeviceLocal);
			vertexBuffer->Upload(vertices.data());
			//MemoryTransfer::Instance().CopyToBuffer(*vertexBuffer, vertices.data());

			indexBuffer = Buffer::Create(
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
				sizeof(indices[0]) * indices.size(), vk::MemoryPropertyFlagBits::eDeviceLocal);
			indexBuffer->Upload(indices.data());
			//MemoryTransfer::Instance().CopyToBuffer(*indexBuffer, indices.data());

			vk::MemoryPropertyFlags hostVisibleFlags;
			{
				using enum vk::MemoryPropertyFlagBits;
				hostVisibleFlags = eHostVisible | eHostCoherent | eDeviceLocal;
			}
			drawCommandsBuffer = Buffer::CreateHostWritable(vk::BufferUsageFlagBits::eIndirectBuffer,
				sizeof(vk::DrawIndexedIndirectCommand) * MAX_FRAMES_IN_FLIGHT * DRAW_BATCH_COUNT, hostVisibleFlags);
			uboStride = Vulkan::PhysicalDevice::Instance().physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
			if (uboStride < sizeof(uint32_t)) // never, but maybe i'll change uint32_t for a larger type
				uboStride = sizeof(uint32_t);
			uniformBuffer = Buffer::CreateHostWritable(vk::BufferUsageFlagBits::eUniformBuffer,
				uboStride * MAX_FRAMES_IN_FLIGHT * DRAW_COUNT, hostVisibleFlags);
			storageBuffer = Buffer::CreateHostWritable(vk::BufferUsageFlagBits::eStorageBuffer,
				sizeof(SSBO) * MAX_FRAMES_IN_FLIGHT * DRAW_COUNT, hostVisibleFlags);

			using namespace Vulkan;
			textureSampler = std::make_shared<Sampler>();
			textureImage = createTextureImage("texture.png");
			textureImage2 = createTextureImage("clown.png");
			textureView = std::make_shared<ImageView>(ImageViewCreateInfo(textureImage, vk::ImageAspectFlagBits::eColor), textureSampler);
			textureView2 = std::make_shared<ImageView>(ImageViewCreateInfo(textureImage2, vk::ImageAspectFlagBits::eColor), textureSampler);

			descriptorSetLayoutGPass = std::make_shared<DescriptorSetLayout>(DescriptorSetLayoutCreateInfo()
				.Binding(0, vk::DescriptorType::eUniformBufferDynamic, vk::ShaderStageFlagBits::eVertex)
				.Binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
				.Binding(2, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, MAX_IMG_ARR_SIZE, true));

			descriptorPool = std::make_shared<DescriptorPool>(DescriptorPoolCreateInfo(DescriptorCounts({
				descriptorSetLayoutGPass }), 1));
			descriptorSet = std::make_shared<DescriptorSet>(DescriptorSetAllocateInfo(descriptorPool, descriptorSetLayoutGPass, MAX_IMG_ARR_SIZE));

			auto writer = descriptorSet->Writer()
				.Buffer(0, *uniformBuffer->Underlying(), 0, sizeof(uint32_t), true)
				.Buffer(1, *storageBuffer->Underlying(), 0, storageBuffer->memory.Size());
			for (size_t i = 0; i < MAX_IMG_ARR_SIZE; i++)
			{
				writer.Image(2, i % 3 ? *textureView2 : *textureView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, i);
			}
			writer.Write();
		}
		std::shared_ptr<Vulkan::Image> createTextureImage(const std::string& path)
		{
			using namespace Vulkan;
			auto image = Renderer::LoadUtils::LoadImage(path, true);

			auto texImg = std::make_shared<Vulkan::Image>(ImageCreateInfo(image.width, image.height, Format::rgba8Unorm,
				vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled), vk::MemoryPropertyFlagBits::eDeviceLocal);

			MemoryTransfer::Instance().CopyToImage(**texImg, image.data.data(), image.data.size(), image.width, image.height);
			return texImg;
		}
		~State()
		{

		}
		void updateUniformBuffer(uint32_t currentFrame)
		{
			using namespace Vulkan;
			static auto startTime = std::chrono::high_resolution_clock::now();

			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

			//glm::mat4 model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			static auto offs = 1.0f;
			static auto pos = glm::vec3(0, 0.5, -1);
			if (Input::Instance().IsDown(Key::Up)) pos.y += 0.1f;
			if (Input::Instance().IsDown(Key::Down)) pos.y -= 0.1f;
			if (Input::Instance().IsDown(Key::Right)) pos.x += 0.1f;
			if (Input::Instance().IsDown(Key::Left)) pos.x -= 0.1f;
			if (Input::Instance().IsDown(Key::W)) pos.z += 0.1f;
			if (Input::Instance().IsDown(Key::S)) pos.z -= 0.1f;
			if (Input::Instance().IsDown(Key::D)) offs *= 0.9f;
			if (Input::Instance().IsDown(Key::A)) offs *= 1.1f;
			auto camPos = glm::translate(glm::identity<glm::mat4>(), pos);
			// model *= glm::scale(glm::identity<glm::mat4>(), glm::vec3(0.1, 0.1, 0.1));
			glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 proj = glm::perspective(glm::radians(45.0f), SwapChain::Instance().Width() / (float)SwapChain::Instance().Height(), 0.1f, 1000.0f);
			//proj[1][1] *= -1;
			vp = proj * view * camPos;
			auto models = &((glm::mat4*)storageBuffer->Ptr())[currentFrame * DRAW_COUNT];
			auto modelIdxs = &((char*)uniformBuffer->Ptr())[currentFrame * DRAW_COUNT * uboStride];
			for (size_t i = 0; i < DRAW_COUNT; i++)
			{
				*(uint32_t*)(&modelIdxs[i * uboStride]) = currentFrame * DRAW_COUNT + i;
				models[i] = glm::translate(glm::identity<glm::mat4>(), glm::vec3(objOffset * i, 0, 0));
			}

			auto drawCmds = (VkDrawIndexedIndirectCommand*)drawCommandsBuffer->Ptr() + DRAW_BATCH_COUNT * currentFrame;
			for (size_t i = 0; i < DRAW_BATCH_COUNT; i++)
			{
				auto& drawCmd = drawCmds[i];
				drawCmd.indexCount = indices.size();
				drawCmd.instanceCount = INSTANCE_COUNT;
				drawCmd.firstIndex = 0;
				drawCmd.vertexOffset = 0;
				drawCmd.firstInstance = i * INSTANCE_COUNT;
			}
		}
	public:
		const std::vector<Vertex> vertices = {
		 {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
		};
		const std::vector<float> quad = {
			-1.0f, -1.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f,

			-1.0f, -1.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 0.0f, 1.0f,
		};
		const std::vector<uint16_t> indices = {
		 0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
		};
		float objOffset = 1.0f;
		std::shared_ptr<Buffer> vertexBuffer;
		std::shared_ptr<Buffer> indexBuffer;

		glm::mat4 vp;
		uint32_t uboStride;
		std::shared_ptr<HostWritableBuffer> uniformBuffer;
		std::shared_ptr<HostWritableBuffer> storageBuffer; // 268 MB is MAX for DeviceLocal HostVisible Coherent
		std::shared_ptr<HostWritableBuffer> drawCommandsBuffer;

		std::shared_ptr<Vulkan::Sampler> textureSampler;
		std::shared_ptr<Vulkan::Image> textureImage;
		std::shared_ptr<Vulkan::Image> textureImage2;
		std::shared_ptr<Vulkan::ImageView> textureView;
		std::shared_ptr<Vulkan::ImageView> textureView2;

		std::shared_ptr<Vulkan::CommandPool> commandPool; // dup

		std::shared_ptr<Vulkan::DescriptorPool> descriptorPool;
		std::shared_ptr<Vulkan::DescriptorSetLayout> descriptorSetLayoutGPass;
		std::shared_ptr<Vulkan::DescriptorSet> descriptorSet;
	};
	class RenderPass_
	{
	public:
		RenderPass_()
		{
			commandPool = std::make_shared<Vulkan::CommandPool>();
		}
		std::vector<uint32_t> LoadShader(const fs::path& srcFile, std::vector<std::array<std::string, 2>> macros = {})
		{
			using namespace Vulkan;
			fs::path binFile = fs::path(srcFile).replace_extension(srcFile.extension().string() + "_bin");
			auto src = FileUtils::ReadAsString(srcFile.string());
			size_t srcHash = std::hash<std::string>()(src);
			if (fs::exists(binFile))
			{ // check if hash is still the same, if so return saved binary
				auto data = FileUtils::ReadBinary<uint32_t>(binFile.string());
				size_t hash = *((size_t*)&data.data()[data.size() - 2]);
				if (srcHash == hash)
				{
					data.resize(data.size() - 2);
					return data;
				}
			}

			using namespace Renderer;
			ShaderType type = ShaderType::None;
			if (srcFile.extension() == ".vert")
				type = ShaderType::Vertex;
			else if (srcFile.extension() == ".frag")
				type = ShaderType::Fragment;
			else
				std::unreachable();

			auto data = Shaderc::Compile(src, type, macros);
			// save with hash
			data.resize(data.size() + 2);
			*((size_t*)&data.data()[data.size() - 2]) = srcHash;
			FileUtils::WriteBinary(data, binFile.string());
			// remove hash
			data.resize(data.size() - 2);
			return data;
		}
		virtual void recordCommandBuffer(Vulkan::CommandBuffer& commandBuffer, uint32_t imageIndex, uint32_t currentFrame, State& state) = 0;
		void BeginRenderPass(vk::RenderPass renderPass, Vulkan::Framebuffer& fb, Vulkan::CommandBuffer& cmd)
		{
			using namespace Vulkan;
			vk::RenderPassBeginInfo beginInfo;
			beginInfo.renderPass = renderPass;
			beginInfo.framebuffer = *fb;
			beginInfo.renderArea.offset = vk::Offset2D();
			beginInfo.renderArea.extent = vk::Extent2D(fb.Width(), fb.Height());

			std::vector<vk::ClearValue> clearValues;
			clearValues.reserve(fb.Attachments().size());
			for (auto& att : fb.Attachments())
			{
				auto format = att->Image()->Format();
				vk::ClearValue clearValue{};
				using enum vk::Format;
				if (format == eD32Sfloat || format == eD16Unorm)
					clearValue.depthStencil = vk::ClearDepthStencilValue(1.0f, 0);
				else if (format == eR8G8B8A8Srgb || format == eR8G8B8A8Unorm)
					clearValue.color = vk::ClearColorValue(std::array<float, 4> {0.0f, 0.0f, 0.0f, 1.0f});
				else
					std::unreachable();
				clearValues.push_back(clearValue);
			}

			beginInfo.clearValueCount = clearValues.size();
			beginInfo.pClearValues = clearValues.data();
			cmd.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
		}
		vk::Viewport ViewportFull(const Vulkan::Framebuffer& fb)
		{
			return vk::Viewport(0, 0, fb.Width(), fb.Height(), 0, 1);
		}
		vk::Rect2D ScissorFull(const Vulkan::Framebuffer& fb)
		{
			return vk::Rect2D({ 0, 0 }, { fb.Width(), fb.Height() });
		}
	public:
		std::shared_ptr<Vulkan::CommandPool> commandPool; // dup
	};
	class DeferredPass : public RenderPass_
	{
	public:
		DeferredPass(State& state)
		{
			createRenderPass();

			using namespace Vulkan;
			auto textureSampler = std::make_shared<Sampler>();
			auto w = SwapChain::Instance().Width();
			auto h = SwapChain::Instance().Height();
			using enum vk::ImageUsageFlagBits;
			using Image = Vulkan::Image;
			auto depthImage_ = std::make_shared<Image>(ImageCreateInfo(w, h, Format::depth16, eDepthStencilAttachment),
				vk::MemoryPropertyFlagBits::eDeviceLocal);
			depthImage = std::make_shared<ImageView>(ImageViewCreateInfo(depthImage_, vk::ImageAspectFlagBits::eDepth), textureSampler);
			auto fbColor_ = std::make_shared<Image>(ImageCreateInfo(w, h, Format::rgba8Unorm, eColorAttachment | eInputAttachment | eTransferSrc | eSampled),
				vk::MemoryPropertyFlagBits::eDeviceLocal);
			fbColor = std::make_shared<ImageView>(ImageViewCreateInfo(fbColor_, vk::ImageAspectFlagBits::eColor), textureSampler);

			auto gPos_ = std::make_shared<Image>(ImageCreateInfo(w, h, Format::rgba8Unorm, eColorAttachment | eInputAttachment),
				vk::MemoryPropertyFlagBits::eDeviceLocal);
			gPos = std::make_shared<ImageView>(ImageViewCreateInfo(gPos_, vk::ImageAspectFlagBits::eColor), textureSampler);
			auto gCol_ = std::make_shared<Image>(ImageCreateInfo(w, h, Format::rgba8Unorm, eColorAttachment | eInputAttachment),
				vk::MemoryPropertyFlagBits::eDeviceLocal);
			gCol = std::make_shared<ImageView>(ImageViewCreateInfo(gCol_, vk::ImageAspectFlagBits::eColor), textureSampler);
			fb = std::make_shared<Framebuffer>(FramebufferCreateInfo(w, h, **renderPass, { gPos, gCol, fbColor, depthImage }));

			descriptorSetLayoutPP = std::make_shared<Vulkan::DescriptorSetLayout>(DescriptorSetLayoutCreateInfo()
				.Binding(0, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
				.Binding(1, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
				.Binding(2, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment));

			descriptorPool = std::make_shared<Vulkan::DescriptorPool>(
				DescriptorPoolCreateInfo(DescriptorCounts({ descriptorSetLayoutPP }), 1));

			descriptorSetPP = std::make_shared<DescriptorSet>(DescriptorSetAllocateInfo(descriptorPool, descriptorSetLayoutPP));
			createGraphicsPipeline(state);

			descriptorSetPP->Writer()
				.Image(0, *gPos, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
				.Image(1, *gCol, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
				.Image(2, *fbColor, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
				.Write();
		}
		void createGraphicsPipeline(State& state)
		{
			using namespace Vulkan;
			ShaderModule vertShaderModule(LoadShader("Source/Shaders/shader.vert",
				{ {"MAX_IMG_ARR_SIZE", std::to_string(MAX_IMG_ARR_SIZE)} }), vk::ShaderStageFlagBits::eVertex);
			ShaderModule fragShaderModule(LoadShader("Source/Shaders/shader.frag"), vk::ShaderStageFlagBits::eFragment);
			vk::PushConstantRange p;
			p.size = sizeof(glm::mat4);
			p.stageFlags = vk::ShaderStageFlagBits::eVertex;
			pipelineLayout = std::make_shared<PipelineLayout>(
				PipelineLayoutCreateInfo({ **state.descriptorSetLayoutGPass,
					**descriptorSetLayoutPP,**descriptorSetLayoutPP }, { p }));
			pipelineGPass = std::make_shared<Pipeline>(PipelineCreateInfo()
				.Layout(**pipelineLayout)
				.AddShader(vertShaderModule)
				.AddShader(fragShaderModule)
				.VertexInput<Vertex>()
				.InputAssembly(vk::PrimitiveTopology::eTriangleList)
				.Rasterization(false) // TRUE
				.Multisampling()
				.ColorBlend(2)
				.RenderPass(**renderPass, 0)
				.DepthStencil(true, true, vk::CompareOp::eLess));

			ShaderModule vertShaderModule3(LoadShader("Source/Shaders/pp.vert"), vk::ShaderStageFlagBits::eVertex);
			ShaderModule fragShaderModule3(LoadShader("Source/Shaders/lPass.frag"), vk::ShaderStageFlagBits::eFragment);
			pipelineLPass = std::make_shared<Pipeline>(PipelineCreateInfo()
				.Layout(**pipelineLayout)
				.AddShader(vertShaderModule3)
				.AddShader(fragShaderModule3)
				.VertexInputHardCoded()
				.InputAssembly(vk::PrimitiveTopology::eTriangleList)
				.Rasterization(false)
				.Multisampling()
				.ColorBlend(1)
				.RenderPass(**renderPass, 1)
				.DepthStencil(false));

			ShaderModule vertShaderModule2(LoadShader("Source/Shaders/pp.vert"), vk::ShaderStageFlagBits::eVertex);
			ShaderModule fragShaderModule2(LoadShader("Source/Shaders/pp.frag"), vk::ShaderStageFlagBits::eFragment);
			pipelinePP = std::make_shared<Pipeline>(PipelineCreateInfo()
				.Layout(**pipelineLayout)
				.AddShader(vertShaderModule2)
				.AddShader(fragShaderModule2)
				.VertexInputHardCoded()
				.InputAssembly(vk::PrimitiveTopology::eTriangleList)
				.Rasterization(false)
				.Multisampling()
				.ColorBlend(1)
				.RenderPass(**renderPass, 2)
				.DepthStencil(false));
		}
		void createRenderPass()
		{
			using namespace Vulkan;
			RenderPassCreateInfo createInfo;

			uint32_t gPosAttIdx = 0, gColAttIdx = 1, lPassAttIdx = 2, depthAttIdx = 3;
			createInfo.Attachments(std::vector<AttachmentDescription>(4));
			createInfo.attachments[gPosAttIdx]
				.setFormat(Format::rgba8Unorm)
				.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal)
				.setLoadOp(vk::AttachmentLoadOp::eClear);
			createInfo.attachments[gColAttIdx] = createInfo.attachments[gPosAttIdx];
			createInfo.attachments[lPassAttIdx]
				.setFormat(Format::rgba8Unorm)
				.setLoadOp(vk::AttachmentLoadOp::eClear)
				.setStoreOp(vk::AttachmentStoreOp::eStore)
				.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
			createInfo.attachments[depthAttIdx]
				.setFormat(Format::depth16)
				.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
				.setLoadOp(vk::AttachmentLoadOp::eClear);

			uint32_t gPass = 0, lPass = 1, ppPass = 2;
			std::vector<SubpassDescription> subPasses(3);
			subPasses[gPass]
				.ColorAttachments({
					{ gPosAttIdx, vk::ImageLayout::eColorAttachmentOptimal },
					{ gColAttIdx, vk::ImageLayout::eColorAttachmentOptimal }
					})
				.DepthAttachment(depthAttIdx);
			subPasses[lPass]
				.ColorAttachments({ { lPassAttIdx, vk::ImageLayout::eColorAttachmentOptimal } })
				.InputAttachments({
					{ gPosAttIdx, vk::ImageLayout::eShaderReadOnlyOptimal },
					{ gColAttIdx, vk::ImageLayout::eShaderReadOnlyOptimal }
					});
			subPasses[ppPass]
				.ColorAttachments({ { lPassAttIdx, vk::ImageLayout::eGeneral } })
				.InputAttachments({ { lPassAttIdx, vk::ImageLayout::eGeneral } });
			createInfo.Subpasses(subPasses);

			createInfo.Dependencies({
				vk::SubpassDependency(VK_SUBPASS_EXTERNAL, gPass)
				.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
				.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
				.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite),

				vk::SubpassDependency(gPass, lPass)
				.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
				.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
				.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
				.setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead)
				.setDependencyFlags(vk::DependencyFlagBits::eByRegion),

				vk::SubpassDependency(lPass, ppPass)
				.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
				.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
				.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
				.setDstAccessMask(vk::AccessFlagBits::eInputAttachmentRead)
				.setDependencyFlags(vk::DependencyFlagBits::eByRegion)
				});
			renderPass = std::make_shared<RenderPass>(createInfo);
		}
		void recordCommandBuffer(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame, State& state) override
		{
			using namespace Vulkan;
			BeginRenderPass(**renderPass, *fb, cmd);
			cmd.setViewport(0, ViewportFull(*fb));
			cmd.setScissor(0, ScissorFull(*fb));

			cmd.pushConstants<glm::mat4>(**pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, state.vp);
			cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipelineGPass);

			cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, **pipelineLayout, 0,
				{ **state.descriptorSet, **descriptorSetPP },
				{ currentFrame * state.uboStride * DRAW_COUNT });

			cmd.bindVertexBuffers(0, **state.vertexBuffer->Underlying(), {0});
			cmd.bindIndexBuffer(**state.indexBuffer->Underlying(), 0, vk::IndexType::eUint16);
			cmd.drawIndexedIndirect(**state.drawCommandsBuffer->Underlying(), sizeof(VkDrawIndexedIndirectCommand) * currentFrame * DRAW_BATCH_COUNT,
				DRAW_BATCH_COUNT, sizeof(vk::DrawIndexedIndirectCommand));

			cmd.nextSubpass(vk::SubpassContents::eInline);
			cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipelineLPass);
			cmd.DrawQuad();

			cmd.nextSubpass(vk::SubpassContents::eInline);
			cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipelinePP);
			cmd.DrawQuad();

			cmd.endRenderPass();
			//    auto builder = CommandBuilder(commandBuffer)
			//        .BeginRenderPass(*fb, *renderPass)
			//        .Viewport().Scissor()
			// 
			//        .BindPipeline(*pipelineGPass)
			//        .PushConstants(*pipelineGPass, VK_SHADER_STAGE_VERTEX_BIT, &state.vp, sizeof(glm::mat4))
			//        .BindVB(*state.vertexBuffer)
			//        .BindIB(*state.indexBuffer, VK_INDEX_TYPE_UINT16)
			//        .BindDescriptorSet(pipelineGPass->layoutHandle, *descriptorSet, { currentFrame * state.uboStride * DRAW_COUNT })
			//        //.DrawIndexed(indices.size(), 1'000'000)
			//        .DrawIndexedIndirect(*state.drawCommandsBuffer, DRAW_BATCH_COUNT, sizeof(VkDrawIndexedIndirectCommand) * currentFrame * DRAW_BATCH_COUNT)
			// 
			//        .NextSubPass()
			//        .BindPipeline(*pipelineLPass)
			//        .BindDescriptorSet(pipelineLPass->layoutHandle, *descriptorSetLPass)
			//        .DrawQuad()
			// 
			//        .NextSubPass()
			//        .BindPipeline(*pipelinePP)
			//        .BindDescriptorSet(pipelinePP->layoutHandle, *descriptorSetPP)
			//        .DrawQuad()
			//        .EndRenderPass();
		}
	public:

		std::shared_ptr<Vulkan::RenderPass> renderPass;

		std::shared_ptr<Vulkan::ImageView> depthImage;
		std::shared_ptr<Vulkan::ImageView> gPos;
		std::shared_ptr<Vulkan::ImageView> gCol;
		std::shared_ptr<Vulkan::ImageView> fbColor;
		std::shared_ptr<Vulkan::Framebuffer> fb;

		std::shared_ptr<Vulkan::DescriptorPool> descriptorPool;
		std::shared_ptr<Vulkan::DescriptorSetLayout> descriptorSetLayoutPP;
		std::shared_ptr<Vulkan::DescriptorSet> descriptorSetPP;

		std::shared_ptr<Vulkan::PipelineLayout> pipelineLayout;
		std::shared_ptr<Vulkan::Pipeline> pipelineGPass;
		std::shared_ptr<Vulkan::Pipeline> pipelineLPass;
		std::shared_ptr<Vulkan::Pipeline> pipelinePP;
	};
	class EditorPass : public RenderPass_
	{
	public:
		EditorPass()
		{
			using namespace Vulkan;
			createRenderPassUI();
			auto w = SwapChain::Instance().Width();
			auto h = SwapChain::Instance().Height();
			auto uiImg = std::make_shared<Vulkan::Image>(ImageCreateInfo(w, h, Format::rgba8Unorm,
				vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc),
				vk::MemoryPropertyFlagBits::eDeviceLocal);
			auto uiImgView = std::make_shared<ImageView>(ImageViewCreateInfo(uiImg, vk::ImageAspectFlagBits::eColor), nullptr);
			fbUI = std::make_shared<Framebuffer>(FramebufferCreateInfo(w, h, **renderPassUI, { uiImgView }));
			//auto imgFormat = Format::rgba8Unorm; //  SwapChain::Instance().createInfo.imageFormat;
			//for (auto& img : SwapChain::Instance().images)
			//{
			//    auto imgView = std::make_shared<ImageView>(ImageViewCreateInfo(img, imgFormat, vk::ImageAspectFlagBits::eColor), nullptr);
			//    auto fb = std::make_shared<Framebuffer>(w, h, *renderPassUI, std::vector<std::shared_ptr<ImageView>>{ imgView });
			//    swapChainfbs.push_back(fb);
			//}
			InitImguiForVulkan();
		}
		~EditorPass()
		{
			ImGui_ImplVulkan_Shutdown();
		}
		void createRenderPassUI()
		{
			using namespace Vulkan;
			RenderPassCreateInfo createInfo;

			uint32_t colAtt = 0;
			createInfo.Attachments(std::vector<AttachmentDescription>(1));
			createInfo.attachments[colAtt]
				.setFormat(Format::rgba8Unorm)
				.setLoadOp(vk::AttachmentLoadOp::eClear)
				.setStoreOp(vk::AttachmentStoreOp::eStore)
				.setFinalLayout(vk::ImageLayout::eTransferSrcOptimal);

			uint32_t sPass = 0;
			createInfo.Subpasses({
				SubpassDescription().ColorAttachments({ { colAtt, vk::ImageLayout::eColorAttachmentOptimal } })
				});

			createInfo.Dependencies({
				vk::SubpassDependency(VK_SUBPASS_EXTERNAL, sPass)
				.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
				.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
				.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite)
				.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite),
				});
			renderPassUI = std::make_shared<RenderPass>(createInfo);
		}
		void InitImguiForVulkan()
		{
			using namespace Vulkan;
			std::vector<vk::DescriptorPoolSize> pool_sizes =
			{
				{ vk::DescriptorType::eSampler, 1000 },
				{ vk::DescriptorType::eCombinedImageSampler, 1000 },
				{ vk::DescriptorType::eSampledImage, 1000 },
				{ vk::DescriptorType::eStorageImage, 1000 },
				{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
				{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
				{ vk::DescriptorType::eUniformBuffer , 1000 },
				{ vk::DescriptorType::eStorageBuffer, 1000 },
				{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
				{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
				{ vk::DescriptorType::eInputAttachment, 1000 }
			};

			imguiPool = std::make_shared<DescriptorPool>(DescriptorPoolCreateInfo(pool_sizes, 1000));

			//this initializes imgui for Vulkan
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = *Instance::StaticInstance();
			init_info.PhysicalDevice = *PhysicalDevice::Instance();
			init_info.Device = *Device::Instance();
			init_info.Queue = *Device::Instance().graphicsQueue;
			init_info.DescriptorPool = **imguiPool;
			init_info.MinImageCount = 3;
			init_info.ImageCount = 3;
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			init_info.Subpass = 0;

			ImGui_ImplVulkan_Init(&init_info, **renderPassUI);

			//execute a gpu command to upload imgui font textures
			Commands::Instance().Submit([&](CommandBuffer& cmd)
				{
					ImGui_ImplVulkan_CreateFontsTexture(*cmd);
				}).OnComplete([]()
					{ //clear font textures from cpu data
						ImGui_ImplVulkan_DestroyFontUploadObjects();
					});
				//.WaitForCompletion();
		}
		void recordCommandBuffer(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame, State& state) override
		{
			using namespace Vulkan;
			BeginRenderPass(**renderPassUI, *fbUI, cmd);
			ImGui::Render();
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *cmd);
			cmd.endRenderPass();

			auto& uiImg = *fbUI->Attachments()[0]->Image();
			cmd.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eTransfer,
				vk::DependencyFlagBits::eByRegion, {}, {}, {
					ImageMemoryBarrier(*uiImg, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eTransferSrcOptimal,
						vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eTransferRead),
					ImageMemoryBarrier(SwapChain::Instance().images[imageIndex], vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
						vk::AccessFlagBits::eNone, vk::AccessFlagBits::eTransferWrite),
				});
			cmd.copyImage(*uiImg, vk::ImageLayout::eTransferSrcOptimal,
				SwapChain::Instance().images[imageIndex], vk::ImageLayout::eTransferDstOptimal,
				vk::ImageCopy(
					vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), {},
					vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), {},
					{ SwapChain::Instance().Width(), SwapChain::Instance().Height(), 1 }
			));
			cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eBottomOfPipe,
				vk::DependencyFlagBits::eByRegion, {}, {}, {
					ImageMemoryBarrier(SwapChain::Instance().images[imageIndex], vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR,
							vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eNone),
				});
		}
		std::shared_ptr<Vulkan::Framebuffer> fbUI;
		std::shared_ptr<Vulkan::DescriptorPool> imguiPool;
		std::shared_ptr<Vulkan::RenderPass> renderPassUI;
	public:
	};
	class RenderLoop
	{
	public:
		RenderLoop()
			: allocator(Vulkan::PhysicalDevice::Instance())
		{
			using namespace Vulkan;
			createSyncObjects();
			commandPool = std::make_shared<CommandPool>();
			commandBuffers = CommandBuffer::Create(commandPool, MAX_FRAMES_IN_FLIGHT);
			renderPasses.push_back(std::make_shared<DeferredPass>(state));
			renderPasses.push_back(std::make_shared<EditorPass>());
		}
		~RenderLoop()
		{
			using namespace Vulkan;
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				vkDestroySemaphore(*Device::Instance(), renderFinishedSemaphores[i], nullptr);
				vkDestroySemaphore(*Device::Instance(), imageAvailableSemaphores[i], nullptr);
				vkDestroyFence(*Device::Instance(), inFlightFences[i], nullptr);
			}
		}
		uint32_t BeginFrame()
		{
			using namespace Vulkan;
			static Timer sumT;
			static float elapsed = 0;

			Timer t;
			vkWaitForFences(*Device::Instance(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
			vkResetFences(*Device::Instance(), 1, &inFlightFences[currentFrame]);
			elapsed += t.Elapsed();

			commandBuffers[currentFrame].reset();
			state.updateUniformBuffer(currentFrame);

			auto result = SwapChain::Instance().acquireNextImage(UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE);
			if (sumT.ElapsedSeconds() >= 1.0f)
			{
				Logger::Info("\t", elapsed, " ms (", 100 * elapsed / sumT.Elapsed(), "%)");
				sumT.Restart();
				elapsed = 0;
			}

			if (result.first == vk::Result::eErrorOutOfDateKHR)
			{
				//swapChain->recreateSwapChain();
				return INVALID_FRAME_IDX;
			}
			else if (result.first != vk::Result::eSuccess && result.first != vk::Result::eSuboptimalKHR)
			{
				throw std::runtime_error("failed to acquire swap chain image!");
			}
			return result.second;
		}
		void EndFrame(uint32_t imgIdx)
		{
			using namespace Vulkan;
			vk::SubmitInfo submitInfo{};

			vk::Semaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
			vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			submitInfo.commandBufferCount = 1;
			submitInfo.setCommandBuffers(*commandBuffers[currentFrame]);

			vk::Semaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			// vkQueueSubmit(*Device::Instance().graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) == VkSuccess();
			Device::Instance().graphicsQueue.submit(submitInfo, inFlightFences[currentFrame]);

			vk::PresentInfoKHR presentInfo;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			vk::SwapchainKHR swapChains[] = { *SwapChain::Instance() };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;
			presentInfo.pImageIndices = &imgIdx;

			presentInfo.pResults = nullptr; // Optional

			auto res = Device::Instance().graphicsQueue.presentKHR(presentInfo);
			if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR || framebufferResized)
			{
				framebufferResized = false;
				//swapChain->recreateSwapChain();
			}
			else if (res != vk::Result::eSuccess)
			{
				throw std::runtime_error("failed to present swap chain image!");
			}
			currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		}
		void DrawFrame()
		{
			using namespace Vulkan;
			Commands::Instance().WaitAll();
			auto imgIdx = BeginFrame();
			if (imgIdx == INVALID_FRAME_IDX)
				return;
			commandBuffers[currentFrame].begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
			for (auto& pass : renderPasses)
			{
				pass->recordCommandBuffer(commandBuffers[currentFrame], imgIdx, currentFrame, state);
			}
			commandBuffers[currentFrame].end();
			EndFrame(imgIdx);
		}
		void createSyncObjects()
		{
			using namespace Vulkan;
			imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
			renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
			inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // create as already signaled (first wait wont deadlock)

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				vkCreateSemaphore(*Device::Instance(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) == VkSuccess();
				vkCreateSemaphore(*Device::Instance(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) == VkSuccess();
				vkCreateFence(*Device::Instance(), &fenceInfo, nullptr, &inFlightFences[i]) == VkSuccess();
			}
		}
	public:
		MemoryAllocator allocator;
		Commands commands;
		MemoryTransfer transfer;
		State state;
		std::vector<std::shared_ptr<RenderPass_>> renderPasses;
		std::vector<Vulkan::CommandBuffer> commandBuffers;
		std::shared_ptr<Vulkan::CommandPool> commandPool;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		uint32_t currentFrame = 0;
		bool framebufferResized = false;
	};

	class Frame
	{
	public:

	public:
	};
}