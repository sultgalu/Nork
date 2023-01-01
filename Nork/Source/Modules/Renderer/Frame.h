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
#include "DeviceResource.h"
#include "Image.h"

namespace Nork::Renderer::Demo {
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
		static const vk::VertexInputBindingDescription getBindingDescription()
		{
			vk::VertexInputBindingDescription bindingDescription;
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = vk::VertexInputRate::eVertex;

			return bindingDescription;
		}
		static const std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions()
		{
			std::vector<vk::VertexInputAttributeDescription> attributeDescriptions(3);

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

	class State
	{
	public:
		State()
		{
			vertexBuffer = Buffer::Create(
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
				sizeof(vertices[0]) * vertices.size(), MemoryFlags{ .required = MemoryFlags::eDeviceLocal });
			// TEST
			// auto vbSize = sizeof(vertices[0]) * vertices.size();
			// uint32_t parts = 5;
			// auto vbSizeTrimmed = vbSize - vbSize % parts;
			// auto remainder = vbSize - vbSizeTrimmed;
			// vk::DeviceSize partSize = vbSizeTrimmed / parts;
			// for (size_t i = 0; i < parts; i++)
			// {
			// 	vk::DeviceSize offset = partSize * i;
			// 	vertexBuffer->Write((uint8_t*)vertices.data() + offset, partSize, offset);
			// }
			// if (remainder > 0)
			// 	vertexBuffer->Write(vertices.data() + vbSizeTrimmed, remainder, vbSizeTrimmed);
			//  vertexBuffer->FlushWrites();
			// TEST
			vertexBuffer->Write(vertices.data());
			vertexBuffer->FlushWrites({}, {});

			auto ibSize = sizeof(indices[0]) * indices.size();
			indexBuffer = Buffer::Create(
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
				ibSize, MemoryFlags{ .required = MemoryFlags::eDeviceLocal });
			indexBuffer->Write((uint8_t*)indices.data(), ibSize, 0);
			indexBuffer->FlushWrites(vk::PipelineStageFlagBits2::eIndexInput, vk::AccessFlagBits2::eIndexRead);

			MemoryFlags hostVisibleFlags;
			{
				using enum vk::MemoryPropertyFlagBits;
				hostVisibleFlags.required = eHostVisible | eHostCoherent | eDeviceLocal;
			}
			drawCommandsBuffer = Buffer::CreateHostWritable(vk::BufferUsageFlagBits::eIndirectBuffer,
				sizeof(vk::DrawIndexedIndirectCommand) * MAX_FRAMES_IN_FLIGHT * DRAW_BATCH_COUNT, hostVisibleFlags);
			uboStride = Vulkan::PhysicalDevice::Instance().physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
			if (uboStride < sizeof(uint32_t)) // never, but maybe i'll change uint32_t for a larger type
				uboStride = sizeof(uint32_t);
			uniformBuffer = Buffer::CreateHostWritable(vk::BufferUsageFlagBits::eUniformBuffer,
				uboStride * MAX_FRAMES_IN_FLIGHT * DRAW_COUNT, hostVisibleFlags);
			storageBuffer = std::make_shared<DeviceElements<SSBO>>(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
				MemoryFlags{ .required = MemoryFlags::eDeviceLocal }, DRAW_COUNT * MAX_FRAMES_IN_FLIGHT, MAX_FRAMES_IN_FLIGHT,
				vk::PipelineStageFlagBits2::eVertexShader, vk::AccessFlagBits2::eShaderStorageRead);
			for (size_t i = 0; i < DRAW_COUNT; i++)
			{
				modelMatrices.push_back(storageBuffer->New());
			}

			using namespace Vulkan;
			texture = createTextureImage("texture.png");
			texture2 = createTextureImage("clown.png");

			descriptorSetLayoutGPass = std::make_shared<DescriptorSetLayout>(DescriptorSetLayoutCreateInfo()
				.Binding(0, vk::DescriptorType::eUniformBufferDynamic, vk::ShaderStageFlagBits::eVertex)
				.Binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
				.Binding(2, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, MAX_IMG_ARR_SIZE, true));

			descriptorPool = std::make_shared<DescriptorPool>(
				DescriptorPoolCreateInfo({ descriptorSetLayoutGPass }, 1));
			descriptorSet = std::make_shared<DescriptorSet>(
				DescriptorSetAllocateInfo(descriptorPool, descriptorSetLayoutGPass, { MAX_IMG_ARR_SIZE }));
			
			textureSampler = std::make_shared<Sampler>();
			auto writer = descriptorSet->Writer()
				.Buffer(0, *uniformBuffer->Underlying(), 0, sizeof(uint32_t), vk::DescriptorType::eUniformBufferDynamic)
				.Buffer(1, *storageBuffer->buffer->Underlying(), 0, storageBuffer->buffer->memory.Size(), vk::DescriptorType::eStorageBuffer);
			for (size_t i = 0; i < MAX_IMG_ARR_SIZE; i++)
			{
				writer.Image(2, i % 3 ? *texture2->view : *texture->view, vk::ImageLayout::eShaderReadOnlyOptimal,
					*textureSampler, vk::DescriptorType::eCombinedImageSampler, i);
			}
			writer.Write();
		}
		std::shared_ptr<Image> createTextureImage(const std::string& path)
		{
			auto image = LoadUtils::LoadImage(path, true);

			auto texImg = std::make_shared<Image>(image.width, image.height, Vulkan::Format::rgba8Unorm,
				vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::ImageAspectFlagBits::eColor,
				vk::PipelineStageFlagBits2::eVertexShader, vk::AccessFlagBits2::eShaderSampledRead);

			texImg->Write(image.data.data(), image.data.size(), vk::ImageLayout::eShaderReadOnlyOptimal);
			return texImg;
		}
		void UpdateSSBO(uint32_t currentFrame)
		{
			static Timer sumT;
			static uint32_t frames = 0;
			static float elapsed = 0;
			static glm::vec3 pos = glm::vec3(0);
			if (Input::Instance().IsDown(Key::Alt)) pos.z -= 0.1f;
			if (Input::Instance().IsDown(Key::Space)) pos.z += 0.1f;
			if (Input::Instance().IsDown(Key::Ctrl)) pos.y -= 0.1f;
			if (Input::Instance().IsDown(Key::M)) pos.y += 0.1f;
			if (Input::Instance().IsDown(Key::E)) pos.x += 0.1f;
			if (Input::Instance().IsDown(Key::Q)) pos.x -= 0.1f;

			for (size_t i = 0; i < DRAW_COUNT; i++)
			{
				SSBO model = SSBO{ glm::translate(glm::identity<glm::mat4>(), pos + glm::vec3(objOffset * i, 0, -10)) };
				auto& mm = modelMatrices[i];
				if (!Input::Instance().IsDown(Key::Shift))
					*mm = model;
					//storageBuffer->buffer->Write(&model, sizeof(model), (currentFrame * DRAW_COUNT + i) * sizeof(model));
			}

			Timer t;
			storageBuffer->FlushWrites();
			// storageBuffer->buffer->FlushWrites(vk::PipelineStageFlagBits2::eVertexShader, vk::AccessFlagBits2::eShaderStorageRead);

			frames++;
			elapsed += t.Elapsed();
			if (sumT.ElapsedSeconds() >= 1.0f)
			{
				Logger::Info("\t", elapsed / frames, " ms (", 100 * elapsed / sumT.Elapsed(), "%)");
				sumT.Restart();
				frames = 0;
				elapsed = 0;
			}
		}
		void updateUniformBuffer(uint32_t currentFrame)
		{
			using namespace Vulkan;
			static auto startTime = std::chrono::high_resolution_clock::now();

			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

			//glm::mat4 model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			static auto front = glm::vec3(0, 0, -1);
			static auto pos = glm::vec3(0, 0.5, 1);
			if (Input::Instance().IsDown(Key::Up)) pos.y += 0.1f;
			if (Input::Instance().IsDown(Key::Down)) pos.y -= 0.1f;
			if (Input::Instance().IsDown(Key::Right)) pos.x += 0.1f;
			if (Input::Instance().IsDown(Key::Left)) pos.x -= 0.1f;
			if (Input::Instance().IsDown(Key::W)) pos.z -= 0.1f;
			if (Input::Instance().IsDown(Key::S)) pos.z += 0.1f;
			if (Input::Instance().IsDown(Key::D)) objOffset *= 0.99f;
			if (Input::Instance().IsDown(Key::A)) objOffset *= 1.01f;
			// auto camPos = glm::translate(glm::identity<glm::mat4>(), pos);
			// model *= glm::scale(glm::identity<glm::mat4>(), glm::vec3(0.1, 0.1, 0.1));
			glm::mat4 view = glm::lookAt(pos, pos + front, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 proj = glm::perspective(glm::radians(45.0f), SwapChain::Instance().Width() / (float)SwapChain::Instance().Height(), 0.1f, 1000.0f);
			proj[1][1] *= -1;
			vp = proj * view;
			auto modelIdxs = &((char*)uniformBuffer->Ptr())[currentFrame * DRAW_COUNT * uboStride];
			for (size_t i = 0; i < DRAW_COUNT; i++)
			{
				*(uint32_t*)(&modelIdxs[i * uboStride]) = currentFrame * DRAW_COUNT + i;
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
		 {{-1, -1, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
		{{1, -1, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
		{{1, 1, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{-1, 1, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},

		{{-1, -1, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
		{{1, -1, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
		{{1, 1, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{1, 1, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
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
		std::shared_ptr<DeviceElements<SSBO>> storageBuffer; // 256 MB is MAX for DeviceLocal HostVisible Coherent
		std::vector<std::shared_ptr<BufferElement<SSBO>>> modelMatrices;
		std::shared_ptr<HostWritableBuffer> drawCommandsBuffer;

		std::shared_ptr<Vulkan::Sampler> textureSampler;
		std::shared_ptr<Image> texture;
		std::shared_ptr<Image> texture2;

		std::shared_ptr<Vulkan::DescriptorPool> descriptorPool;
		std::shared_ptr<Vulkan::DescriptorSetLayout> descriptorSetLayoutGPass;
		std::shared_ptr<Vulkan::DescriptorSet> descriptorSet;
	};
	class RenderPass_
	{
	public:
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
	};
	class DeferredPass : public RenderPass_
	{
	public:
		DeferredPass(State& state)
		{
			createRenderPass();

			auto w = Vulkan::SwapChain::Instance().Width();
			auto h = Vulkan::SwapChain::Instance().Height();
			using enum vk::ImageUsageFlagBits;
			depthImage = std::make_shared<Image>(w, h, Vulkan::Format::depth16, eDepthStencilAttachment, vk::ImageAspectFlagBits::eDepth);
			fbColor = std::make_shared<Image>(w, h, Vulkan::Format::rgba8Unorm, eColorAttachment | eInputAttachment | eTransferSrc | eSampled,
				vk::ImageAspectFlagBits::eColor);
			gPos = std::make_shared<Image>(w, h, Vulkan::Format::rgba8Unorm, eColorAttachment | eInputAttachment, vk::ImageAspectFlagBits::eColor);
			gCol = std::make_shared<Image>(w, h, Vulkan::Format::rgba8Unorm, eColorAttachment | eInputAttachment, vk::ImageAspectFlagBits::eColor);
			fb = std::make_shared<Vulkan::Framebuffer>(Vulkan::FramebufferCreateInfo(w, h, **renderPass, { gPos->view, gCol->view, fbColor->view, depthImage->view }));

			using namespace Vulkan;
			descriptorSetLayoutPP = std::make_shared<Vulkan::DescriptorSetLayout>(Vulkan::DescriptorSetLayoutCreateInfo()
				.Binding(0, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
				.Binding(1, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
				.Binding(2, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment));

			descriptorPool = std::make_shared<Vulkan::DescriptorPool>(
				DescriptorPoolCreateInfo({ descriptorSetLayoutPP }, 1 ));

			descriptorSetPP = std::make_shared<DescriptorSet>(DescriptorSetAllocateInfo(descriptorPool, descriptorSetLayoutPP));
			createGraphicsPipeline(state);

			auto textureSampler = std::make_shared<Sampler>();
			descriptorSetPP->Writer()
				.Image(0, *gPos->view, vk::ImageLayout::eShaderReadOnlyOptimal, *textureSampler, vk::DescriptorType::eInputAttachment)
				.Image(1, *gCol->view, vk::ImageLayout::eShaderReadOnlyOptimal, *textureSampler, vk::DescriptorType::eInputAttachment)
				.Image(2, *fbColor->view, vk::ImageLayout::eGeneral, *textureSampler, vk::DescriptorType::eInputAttachment)
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
				.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setStoreOp(vk::AttachmentStoreOp::eStore);
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
				.setDependencyFlags(vk::DependencyFlagBits::eByRegion),
				
				vk::SubpassDependency(ppPass, VK_SUBPASS_EXTERNAL)
				.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
				.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
				.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
				.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
				});
			renderPass = std::make_shared<Vulkan::RenderPass>(createInfo);
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

			cmd.bindVertexBuffers(0, **state.vertexBuffer->Underlying(), { 0 });
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
		}
	public:

		std::shared_ptr<Vulkan::RenderPass> renderPass;

		std::shared_ptr<Image> depthImage;
		std::shared_ptr<Image> gPos;
		std::shared_ptr<Image> gCol;
		std::shared_ptr<Image> fbColor;
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
			createRenderPassUI();
			auto w = Vulkan::SwapChain::Instance().Width();
			auto h = Vulkan::SwapChain::Instance().Height();
			imgUI = std::make_shared<Image>(w, h, Vulkan::Format::rgba8Unorm,
				vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
				vk::ImageAspectFlagBits::eColor);
			fbUI = std::make_shared<Vulkan::Framebuffer>(Vulkan::FramebufferCreateInfo(w, h, **renderPassUI, { imgUI->view }));
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
				vk::SubpassDependency(sPass, VK_SUBPASS_EXTERNAL)
				.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
				.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
				.setDstStageMask(vk::PipelineStageFlagBits::eTransfer)
				.setDstAccessMask(vk::AccessFlagBits::eTransferRead),
				});
			renderPassUI = std::make_shared<Vulkan::RenderPass>(createInfo);
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
			Commands::Instance().TransferCommand([&](CommandBuffer& cmd)
				{
					ImGui_ImplVulkan_CreateFontsTexture(*cmd);
				});
			Commands::Instance().OnTransfersFinished([]()
				{ //clear font textures from cpu data
					ImGui_ImplVulkan_DestroyFontUploadObjects();
				});
		}
		void recordCommandBuffer(Vulkan::CommandBuffer& cmd, uint32_t imageIndex, uint32_t currentFrame, State& state) override
		{
			using namespace Vulkan;
			BeginRenderPass(**renderPassUI, *fbUI, cmd);
			ImGui::Render();
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *cmd);
			cmd.endRenderPass();

			auto& uiImg = *fbUI->Attachments()[0]->Image();
			auto barrierSc = vk::ImageMemoryBarrier2()
				.setImage(SwapChain::Instance().images[imageIndex])
				.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))

				.setOldLayout(vk::ImageLayout::eUndefined)
				.setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
				.setSrcAccessMask(vk::AccessFlagBits2::eNone)

				.setNewLayout(vk::ImageLayout::eTransferDstOptimal)
				.setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
				.setDstAccessMask(vk::AccessFlagBits2::eTransferWrite);

			cmd.pipelineBarrier2(vk::DependencyInfo(vk::DependencyFlagBits::eByRegion)
				.setImageMemoryBarriers(barrierSc));

			cmd.copyImage(*uiImg, vk::ImageLayout::eTransferSrcOptimal,
				SwapChain::Instance().images[imageIndex], vk::ImageLayout::eTransferDstOptimal,
				vk::ImageCopy(
					vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), {},
					vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), {},
					{ SwapChain::Instance().Width(), SwapChain::Instance().Height(), 1 }
			));

			barrierSc
				.setOldLayout(barrierSc.newLayout)
				.setSrcStageMask(barrierSc.dstStageMask)
				.setSrcAccessMask(barrierSc.dstAccessMask)

				.setNewLayout(vk::ImageLayout::ePresentSrcKHR)
				.setDstStageMask(vk::PipelineStageFlagBits2::eBottomOfPipe)
				.setDstAccessMask(vk::AccessFlagBits2::eNone);
			cmd.pipelineBarrier2(vk::DependencyInfo().setImageMemoryBarriers(barrierSc));
		}
		std::shared_ptr<Image> imgUI;
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
			//Commands::Instance().Begin(0);

			state = std::make_unique<State>();
			createSyncObjects();
			renderPasses.push_back(std::make_shared<DeferredPass>(*state));
			renderPasses.push_back(std::make_shared<EditorPass>());

			//Commands::Instance().End();
			//Commands::Instance().Submit();
		}
		~RenderLoop()
		{
			using namespace Vulkan;
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				vkDestroySemaphore(*Device::Instance(), imageAvailableSemaphores[i], nullptr);
			}
		}
		uint32_t BeginFrame()
		{
			using namespace Vulkan;

			Commands::Instance().NextFrame(currentFrame);
			Commands::Instance().BeginTransferCommandBuffer();
			Commands::Instance().BeginRenderCommandBuffer();
			state->storageBuffer->OnNewFrame();
			state->updateUniformBuffer(currentFrame);
			state->UpdateSSBO(currentFrame);

			auto result = SwapChain::Instance().acquireNextImage(UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE);
			
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

			Commands::Instance().SubmitTransferCommands();
			Commands::Instance().SubmitRenderCommands(true, { imageAvailableSemaphores[currentFrame] });
			
			auto res = Device::Instance().graphicsQueue.presentKHR(
				vk::PresentInfoKHR()
				.setWaitSemaphores(**Commands::Instance().renderFinishedSemaphores2[currentFrame])
				.setSwapchains(*SwapChain::Instance())
				.setImageIndices(imgIdx)
			);
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
			auto imgIdx = BeginFrame();
			if (imgIdx == INVALID_FRAME_IDX)
				return;
			for (auto& pass : renderPasses)
			{
				pass->recordCommandBuffer(*Commands::Instance().renderCmds[currentFrame], imgIdx, currentFrame, *state);
			}
			Commands::Instance().EndTransferCommandBuffer();
			Commands::Instance().EndRenderCommandBuffer();
			EndFrame(imgIdx);
		}
		void createSyncObjects()
		{
			using namespace Vulkan;
			imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // create as already signaled (first wait wont deadlock)

			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
			{
				vkCreateSemaphore(*Device::Instance(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) == VkSuccess();
			}
		}
	public:
		MemoryAllocator allocator;
		Commands commands = Commands(MAX_FRAMES_IN_FLIGHT);
		MemoryTransfer transfer;
		std::unique_ptr<State> state;
		std::vector<std::shared_ptr<RenderPass_>> renderPasses;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		uint32_t currentFrame = 0;
		bool framebufferResized = false;
	};
}